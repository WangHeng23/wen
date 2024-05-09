#include "ray_tracing/acceleration_structure.hpp"
#include "utils/utils.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

namespace wen {

AccelerationStructure::~AccelerationStructure() {
    staging_.reset();
    scratch_.reset();
}

void AccelerationStructure::addModel(std::shared_ptr<RayTracingModel> model) {
    auto type = model->getType();
    switch (type) {
        case RayTracingModel::ModelType::eNormalModel:
            models_.push_back(std::move(std::dynamic_pointer_cast<Model>(model)));
            break;
        case RayTracingModel::ModelType::eGLTFPrimitive:
            WEN_ERROR("GLTFPrimitive is used to addScene!")
            break;
    }
}

void AccelerationStructure::addScene(std::shared_ptr<RayTracingScene> scene) {
    auto type = scene->getType();
    switch (type) {
        case RayTracingScene::SceneType::eGLTFScene:
            scenes_.push_back(std::move(std::dynamic_pointer_cast<GLTFScene>(scene)));
            break;
    }
}

void AccelerationStructure::build(bool is_update, bool allow_update) {
    std::vector<AccelerationStructureInfo> infos;
    infos.reserve(models_.size() + scenes_.size());

    uint64_t maxStagingSize = currentStagingSize_, maxScratchSize = currentScratchSize_;

    auto mode = is_update ? vk::BuildAccelerationStructureModeKHR::eUpdate
                          : vk::BuildAccelerationStructureModeKHR::eBuild;
    auto flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction |
                 vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    if (allow_update) {
        flags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
    }

    // 将模型转变成光追几何体用于构建底层加速结构
    for (auto& model : models_) {
        if (!is_update) {
            uint64_t verticesSize = model->vertexCount * sizeof(Vertex);
            uint64_t indicesSize = model->indexCount * sizeof(uint32_t);
            model->rayTracingVertexBuffer = std::make_unique<Buffer>(
                verticesSize,
                // 将数据或显存从其他缓冲区复制到此缓冲区
                vk::BufferUsageFlagBits::eTransferDst |
                    // 用于加速结构的只读输入
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    // 用于在着色器中获取这个缓冲的地址来访问数据
                    vk::BufferUsageFlagBits::eShaderDeviceAddress,
                VMA_MEMORY_USAGE_GPU_ONLY,
                // 为此次分配创建专用的内存
                VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
            );
            model->rayTracingIndexBuffer = std::make_unique<Buffer>(
                indicesSize,
                vk::BufferUsageFlagBits::eTransferDst |
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    vk::BufferUsageFlagBits::eShaderDeviceAddress,
                VMA_MEMORY_USAGE_GPU_ONLY,
                VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
            );
            maxStagingSize = std::max(maxStagingSize, verticesSize);
            maxStagingSize = std::max(maxStagingSize, indicesSize);
            model->modelAs = std::make_unique<ModelAccelerationStructure>();
        }

        auto& info = infos.emplace_back(*(model->modelAs.value()));
        uint32_t maxPrimitiveCount = model->indexCount / 3;

        // 将缓存描述为 VertexObj
        info.geometryDatas.emplace_back().triangles
            // 顶点的位置数据 : glm::vec3
            .setVertexFormat(vk::Format::eR32G32B32Sfloat)
            // 顶点数据的原内存地址
            .setVertexData(getBufferAddress(model->rayTracingVertexBuffer->buffer))
            // 顶点数据的偏移
            .setVertexStride(sizeof(Vertex))
            // 索引数据 : uint32_t
            .setIndexType(vk::IndexType::eUint32)
            .setIndexData(getBufferAddress(model->rayTracingIndexBuffer->buffer))
            .setMaxVertex(model->vertexCount - 1)
            .sType = vk::StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
        // 将三角形设置为不透明
        info.geometries.emplace_back()
            .setGeometry(info.geometryDatas[0])
            .setGeometryType(vk::GeometryTypeKHR::eTriangles)
            .setFlags(vk::GeometryFlagBitsKHR::eOpaque);
        // 设置偏移
        info.offsets.emplace_back()
            .setFirstVertex(0)
            .setPrimitiveCount(maxPrimitiveCount)
            .setPrimitiveOffset(0)
            .setTransformOffset(0);
        // 设置构建信息
        info.build
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setMode(mode)
            .setFlags(flags)
            .setGeometries(info.geometries);
        info.size = manager->device->device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice,
            info.build, maxPrimitiveCount, manager->dispatcher
        );
        maxScratchSize = std::max(maxScratchSize, is_update ? info.size.updateScratchSize : info.size.buildScratchSize);
    }
    // 将GLTF场景转变成光追几何体用于构建底层加速结构
    for (auto& scene : scenes_) {
        if (!is_update) {
            uint64_t verticesSize = scene->vertices.size() * sizeof(glm::vec3);
            uint64_t indicesSize = scene->indices.size() * sizeof(uint32_t);
            scene->rayTracingVertexBuffer = std::make_unique<Buffer>(
                verticesSize,
                vk::BufferUsageFlagBits::eTransferDst |
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    vk::BufferUsageFlagBits::eShaderDeviceAddress,
                VMA_MEMORY_USAGE_GPU_ONLY,
                VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
            );
            scene->rayTracingIndexBuffer = std::make_unique<Buffer>(
                indicesSize,
                vk::BufferUsageFlagBits::eTransferDst |
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    vk::BufferUsageFlagBits::eShaderDeviceAddress,
                VMA_MEMORY_USAGE_GPU_ONLY,
                VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
            );
            maxStagingSize = std::max(maxStagingSize, verticesSize);
            maxStagingSize = std::max(maxStagingSize, indicesSize);
        }

        scene->build([&](auto* node, auto primitive) {
            if (!is_update) {
                primitive->modelAs = std::make_unique<ModelAccelerationStructure>();
            }
            auto& info = infos.emplace_back(*primitive->modelAs.value());
            uint32_t primitiveCount = primitive->indexCount / 3;

            info.geometryDatas.emplace_back().triangles
                .setVertexFormat(vk::Format::eR32G32B32Sfloat)
                .setVertexData(getBufferAddress(scene->rayTracingVertexBuffer->buffer))
                .setVertexStride(sizeof(glm::vec3))
                .setIndexType(vk::IndexType::eUint32)
                .setIndexData(getBufferAddress(scene->rayTracingIndexBuffer->buffer))
                .setMaxVertex(primitive->vertexCount - 1)
                .sType = vk::StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
            info.geometries.emplace_back()
                .setGeometry(info.geometryDatas.back())
                .setGeometryType(vk::GeometryTypeKHR::eTriangles)
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque);
            info.offsets.emplace_back()
                .setFirstVertex(primitive->data().firstVertex)
                .setPrimitiveCount(primitiveCount)
                .setPrimitiveOffset(primitive->data().firstIndex * sizeof(uint32_t))
                .setTransformOffset(0);
            info.build
                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                .setMode(mode)
                .setFlags(flags)
                .setGeometries(info.geometries);
            info.size = manager->device->device.getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice,
                info.build, primitiveCount, manager->dispatcher
            );
            maxScratchSize = std::max(maxScratchSize, is_update ? info.size.updateScratchSize : info.size.buildScratchSize);
        });
    }

    // create staging and scratch buffers if needed.
    if (maxStagingSize > currentStagingSize_) {
        staging_.reset();
        staging_ = std::make_unique<StorageBuffer>(
            maxStagingSize,
            // 可以将这个缓冲区的数据复制到其他缓冲区或显存
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            // 在CPU端对分配的内存进行顺序写入
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                // 允许使用传输队列执行传输操作
                VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
        );
        currentStagingSize_ = maxStagingSize;
    }
    if (maxScratchSize > currentScratchSize_) {
        scratch_.reset();
        scratch_ = std::make_unique<StorageBuffer>(
            maxScratchSize,
            // 存储缓冲区是一种特殊类型的缓冲区，它可以在着色器中被读写
            vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
            VMA_MEMORY_USAGE_GPU_ONLY,
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
        );
        currentScratchSize_ = maxScratchSize;
    }

    // upload data to gpu
    if (!is_update) {
        for (auto& model : models_) {
            auto* ptr = static_cast<uint8_t*>(staging_->map());

            uint64_t verticesSize = model->vertexCount * sizeof(Vertex);
            memcpy(ptr, model->vertices().data(), verticesSize);
            staging_->flush(verticesSize, model->rayTracingVertexBuffer->buffer);

            uint64_t indicesSize = 0;
            for (const auto& [name, mesh] : model->meshes()) {
                uint64_t size = mesh->indices.size() * sizeof(uint32_t);
                memcpy(ptr, mesh->indices.data(), size);
                ptr += size;
                indicesSize += size;
            }
            staging_->flush(indicesSize, model->rayTracingIndexBuffer->buffer);
        }
        for (auto& scene : scenes_) {
            auto* ptr = static_cast<uint8_t*>(staging_->map());

            uint64_t verticesSize = scene->vertices.size() * sizeof(glm::vec3);
            memcpy(ptr, scene->vertices.data(), verticesSize);
            staging_->flush(verticesSize, scene->rayTracingVertexBuffer->buffer);

            uint64_t indicesSize = scene->indices.size() * sizeof(uint32_t);
            memcpy(ptr, scene->indices.data(), indicesSize);
            staging_->flush(indicesSize, scene->rayTracingIndexBuffer->buffer);
        }
        if (staging_.get() != nullptr) {
            staging_->unmap();
        }
    }

    // 创建一个用于获取每一个BLAS压缩的存储大小的查询队列
    vk::QueryPoolCreateInfo info = {};
    info.setQueryCount(infos.size())
        .setQueryType(vk::QueryType::eAccelerationStructureCompactedSizeKHR);
    auto queryPool = manager->device->device.createQueryPool(info);

    // 批量创建/压缩底层加速结构，这样可以存入有限的内存
    std::vector<uint32_t> indices;
    vk::DeviceSize batchSize = {0};
    vk::DeviceSize batchLimit = {256'000'000}; // 256 MB
    for (int i = 0; i < infos.size(); i++) {
        // indices数组用于限值一次性创建底层加速结构的数量
        indices.push_back(i);
        batchSize += infos[i].size.accelerationStructureSize;
        // 超过256MB或是最后一个底层加速结构
        if (batchSize > batchLimit || i == infos.size() - 1) {
            auto cmdbuf = manager->commandPool->allocateSingleUse();
            cmdbuf.resetQueryPool(queryPool, 0, infos.size());
            if (!is_update) {
                uint32_t index = 0;
                for (auto j : indices) {
                    auto& info = infos[j];
                    // 真正的缓存分配和加速结构创建
                    info.buffer = std::make_unique<StorageBuffer>(
                        info.size.accelerationStructureSize,
                        // 可以将这个缓冲区的数据用于存储加速结构的数据
                        vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                            vk::BufferUsageFlagBits::eShaderDeviceAddress,
                        VMA_MEMORY_USAGE_GPU_ONLY,
                        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
                    );
                    vk::AccelerationStructureCreateInfoKHR createInfo = {};
                    createInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                              .setBuffer(info.buffer->getBuffer())
                              // 将用于内存分配
                              .setSize(info.size.accelerationStructureSize);
                    info.as = manager->device->device.createAccelerationStructureKHR(createInfo, nullptr, manager->dispatcher);

                    // 构建底层加速结构
                    info.build
                        .setDstAccelerationStructure(info.as)
                        .setScratchData(getBufferAddress(scratch_->getBuffer()));
                    cmdbuf.buildAccelerationStructuresKHR(info.build, info.offsets.data(), manager->dispatcher);

                    // 一旦暂付缓存被重复使用, 我们需要一个栅栏用于确保之前的构建已经结束才开始构建下一个
                    vk::MemoryBarrier barrier = {};
                    barrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR)
                           .setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);
                    cmdbuf.pipelineBarrier(
                        vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                        vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                        vk::DependencyFlags{},
                        barrier,
                        {},
                        {}
                    );
                    // 查询真正需要的内存数量，用于压缩
                    cmdbuf.writeAccelerationStructuresPropertiesKHR(
                        info.as,
                        vk::QueryType::eAccelerationStructureCompactedSizeKHR,
                        queryPool, index, manager->dispatcher
                    );
                    index++;
                }
                manager->commandPool->freeSingleUse(cmdbuf);

                /*
                    大体上来说，压缩流程如下：
                    1. 获取查询到的数据（压缩大小）
                    2. 使用较小的大小创建一个新的加速结构
                    3. 将之前的加速结构拷贝到新创建的加速结构中
                    4. 将之前的加速结构销毁
                */
                // 获取查询到的数据（压缩大小）
                std::vector<vk::DeviceSize> compactSizes(infos.size());
                auto res = manager->device->device.getQueryPoolResults(
                    queryPool,
                    0,
                    compactSizes.size(),
                    sizeof(vk::DeviceSize) * compactSizes.size(),
                    compactSizes.data(),
                    sizeof(vk::DeviceSize),
                    vk::QueryResultFlagBits::eWait
                );
                WEN_ASSERT(res == vk::Result::eSuccess, "Failed to get query pool results.")
                
                cmdbuf = manager->commandPool->allocateSingleUse();
                index = 0;
                for (auto j : indices) {
                    auto& info = infos[j];
                    // 创建压缩版本的加速结构
                    auto& modelAS = info.modelAs;
                    modelAS.buffer = std::make_unique<StorageBuffer>(
                        compactSizes[index],
                        vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                            vk::BufferUsageFlagBits::eShaderDeviceAddress,
                        VMA_MEMORY_USAGE_GPU_ONLY,
                        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
                    );
                    vk::AccelerationStructureCreateInfoKHR createInfo = {};
                    createInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                              .setBuffer(modelAS.buffer->getBuffer())
                              .setSize(compactSizes[index]);
                    modelAS.blas = manager->device->device.createAccelerationStructureKHR(createInfo, nullptr, manager->dispatcher);

                    // 将之前的底层加速结构拷贝至压缩版本中
                    vk::CopyAccelerationStructureInfoKHR compact = {};
                    compact.setSrc(info.as)
                           .setDst(modelAS.blas)
                           .setMode(vk::CopyAccelerationStructureModeKHR::eCompact);
                    cmdbuf.copyAccelerationStructureKHR(compact, manager->dispatcher);
                    index++;
                }
            } else {
                for (auto j : indices) {
                    auto& info = infos[j];
                    auto& modelAS = info.modelAs;
                    info.build
                        .setSrcAccelerationStructure(modelAS.blas)
                        .setDstAccelerationStructure(modelAS.blas)
                        .setScratchData(getBufferAddress(scratch_->getBuffer()));
                    cmdbuf.buildAccelerationStructuresKHR(info.build, info.offsets.data(), manager->dispatcher);

                    vk::MemoryBarrier barrier = {};
                    barrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR)
                           .setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);
                    cmdbuf.pipelineBarrier(
                        vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                        vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                        vk::DependencyFlags{},
                        barrier,
                        {},
                        {}
                    );
                }
            }
            manager->commandPool->freeSingleUse(cmdbuf);

            // 重置
            batchSize = 0;
            indices.clear();
        }
    }

    if (!is_update) {
        for (auto& info : infos) {
            manager->device->device.destroyAccelerationStructureKHR(info.as, nullptr, manager->dispatcher);
            info.buffer.reset();
        }
    }
    infos.clear();
    manager->device->device.destroyQueryPool(queryPool);
    models_.clear();
    scenes_.clear();
}

} // namespace wen