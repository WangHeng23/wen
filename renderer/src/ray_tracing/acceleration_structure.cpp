#include "ray_tracing/acceleration_structure.hpp"
#include "utils/utils.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

namespace wen {

ModelAccelerationStructure::~ModelAccelerationStructure() {
    manager->device->device.destroyAccelerationStructureKHR(blas, nullptr, manager->dispatcher);
    buffer.reset();
}

AccelerationStructure::~AccelerationStructure() {
    staging_.reset();
    scratch_.reset();
    models_.clear();
}

void AccelerationStructure::addModel(std::shared_ptr<RayTracingModel> model) {
    auto type = model->getType();
    switch (type) {
        case RayTracingModel::ModelType::eNormalModel:
            models_.push_back(std::move(std::dynamic_pointer_cast<Model>(model)));
            break;
    }
}

void AccelerationStructure::build(bool is_update, bool allow_update) {
    std::vector<AccelerationStructureInfo> infos;
    infos.reserve(models_.size());

    uint64_t maxStagingSize = currentStagingSize_, maxScratchSize = currentScratchSize_;

    auto mode = is_update ? vk::BuildAccelerationStructureModeKHR::eUpdate
                          : vk::BuildAccelerationStructureModeKHR::eBuild;
    auto flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction |
                 vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    if (allow_update) {
        flags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
    }

    for (auto& model : models_) {
        if (!is_update) {
            uint64_t verticesSize = model->vertexCount * sizeof(Vertex);
            uint64_t indicesSize = model->indexCount * sizeof(uint32_t);
            model->vertexBuffer = std::make_unique<Buffer>(
                verticesSize,
                vk::BufferUsageFlagBits::eTransferDst |
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    vk::BufferUsageFlagBits::eShaderDeviceAddress,
                VMA_MEMORY_USAGE_GPU_ONLY,
                VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
            );
            model->indexBuffer = std::make_unique<Buffer>(
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

        auto& info = infos.emplace_back(*model->modelAs.value());
        uint32_t maxPrimitiveCount = model->indexCount / 3;

        // Describe buffer as array of VertexObj.
        info.geometryDatas.emplace_back().triangles
            // vec3 vertex position data.
            .setVertexFormat(vk::Format::eR32G32B32Sfloat)
            .setVertexData(getBufferAddress(model->vertexBuffer->getBuffer()))
            .setVertexStride(sizeof(Vertex))
            // Describe index data (32-bit unsigned int)
            .setIndexType(vk::IndexType::eUint32)
            .setIndexData(getBufferAddress(model->indexBuffer->getBuffer()))
            .setMaxVertex(model->vertexCount - 1)
            .sType = vk::StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
        // Identify the above data as containing opaque triangles.
        info.geometries.emplace_back()
            .setGeometry(info.geometryDatas[0])
            .setGeometryType(vk::GeometryTypeKHR::eTriangles)
            .setFlags(vk::GeometryFlagBitsKHR::eOpaque);
        // The entire array will be used to build the BLAS.
        info.offsets.emplace_back()
            .setFirstVertex(0)
            .setPrimitiveCount(maxPrimitiveCount)
            .setPrimitiveOffset(0)
            .setTransformOffset(0);
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

    // Create staging and scratch buffers if needed.
    if (maxStagingSize > currentStagingSize_) {
        staging_.reset();
        staging_ = std::make_unique<Buffer>(
            maxStagingSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
        );
        currentStagingSize_ = maxStagingSize;
    }
    if (maxScratchSize > currentScratchSize_) {
        scratch_.reset();
        scratch_ = std::make_unique<Buffer>(
            maxScratchSize,
            vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
            VMA_MEMORY_USAGE_GPU_ONLY,
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
        );
    }

    // Copy vertex and index data to staging buffer.
    if (!is_update) {
        for (auto& model : models_) {
            auto* ptr = static_cast<uint8_t*>(staging_->map());

            uint64_t verticesSize = model->vertexCount * sizeof(Vertex);
            memcpy(ptr, model->vertices().data(), verticesSize);
            auto cmdbuf = manager->commandPool->allocateSingleUse();
            vk::BufferCopy copy{};
            copy.setSrcOffset(0)
                .setDstOffset(0)
                .setSize(verticesSize);
            cmdbuf.copyBuffer(staging_->getBuffer(), model->vertexBuffer->getBuffer(), copy);
            manager->commandPool->freeSingleUse(cmdbuf);

            uint64_t indicesSize = 0;
            for (const auto& [name, mesh] : model->meshes()) {
                uint64_t size = mesh->indices.size() * sizeof(uint32_t);
                memcpy(ptr, mesh->indices.data(), size);
                ptr += size;
                indicesSize += size;
            }
            cmdbuf = manager->commandPool->allocateSingleUse();
            copy.setSize(indicesSize);
            cmdbuf.copyBuffer(staging_->getBuffer(), model->indexBuffer->getBuffer(), copy);
            manager->commandPool->freeSingleUse(cmdbuf);
        }
        if (staging_.get() != nullptr) {
            staging_->unmap();
        }
    }

    // Create query pool to get compacted size of BLAS.
    vk::QueryPoolCreateInfo info = {};
    info.setQueryCount(infos.size())
        .setQueryType(vk::QueryType::eAccelerationStructureCompactedSizeKHR);
    auto queryPool = manager->device->device.createQueryPool(info);

    // Batching creation/compaction of BLAS to allow staying in restricted amount of memory
    std::vector<uint32_t> buildInfoIndices;
    vk::DeviceSize batchSize = {0};
    vk::DeviceSize batchLimit = {256'000'000}; // 256 MB
    for (int i = 0; i < infos.size(); i++) {
        buildInfoIndices.push_back(i);
        batchSize += infos[i].size.accelerationStructureSize;
        // Over the limit or last BLAS element
        if (batchSize > batchLimit || i == infos.size() - 1) {
            auto cmdbuf = manager->commandPool->allocateSingleUse();
            cmdbuf.resetQueryPool(queryPool, 0, infos.size());
            if (!is_update) {
                uint32_t index = 0;
                for (auto j : buildInfoIndices) {
                    auto& info = infos[j];
                    // Actual allocation of buffer and acceleration structure.
                    info.buffer = std::make_unique<Buffer>(
                        info.size.accelerationStructureSize,
                        vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                            vk::BufferUsageFlagBits::eShaderDeviceAddress,
                        VMA_MEMORY_USAGE_GPU_ONLY,
                        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
                    );
                    vk::AccelerationStructureCreateInfoKHR createInfo{};
                    createInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                              .setBuffer(info.buffer->getBuffer())
                              .setSize(info.size.accelerationStructureSize);
                    info.as = manager->device->device.createAccelerationStructureKHR(createInfo, nullptr, manager->dispatcher);

                    info.build
                        .setDstAccelerationStructure(info.as)
                        .setScratchData(getBufferAddress(scratch_->getBuffer()));

                    // Building the bottom-level-acceleration-structure
                    cmdbuf.buildAccelerationStructuresKHR(info.build, info.offsets.data(), manager->dispatcher);

                    vk::MemoryBarrier barrier{};
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
                    // Add a query to find the 'real' amount of memory needed, use for compaction
                    cmdbuf.writeAccelerationStructuresPropertiesKHR(
                        info.as,
                        vk::QueryType::eAccelerationStructureCompactedSizeKHR,
                        queryPool, index, manager->dispatcher
                    );
                    index++;
                }
                manager->commandPool->freeSingleUse(cmdbuf);

                // Get the compacted size result back
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
                for (auto j : buildInfoIndices) {
                    auto& info = infos[j];
                    // Creating a compact version of the AS
                    auto& modelAS = info.modelAs;
                    modelAS.buffer = std::make_unique<Buffer>(
                        compactSizes[index],
                        vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                            vk::BufferUsageFlagBits::eShaderDeviceAddress,
                        VMA_MEMORY_USAGE_GPU_ONLY,
                        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
                    );
                    vk::AccelerationStructureCreateInfoKHR createInfo{};
                    createInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                              .setBuffer(modelAS.buffer->getBuffer())
                              .setSize(compactSizes[index]);
                    modelAS.blas = manager->device->device.createAccelerationStructureKHR(createInfo, nullptr, manager->dispatcher);

                    // Copy the original BLAS to a compact version
                    vk::CopyAccelerationStructureInfoKHR compact{};
                    compact.setSrc(info.as)
                           .setDst(modelAS.blas)
                           .setMode(vk::CopyAccelerationStructureModeKHR::eCompact);
                    cmdbuf.copyAccelerationStructureKHR(compact, manager->dispatcher);
                    index++;
                }
            } else {
                for (auto j : buildInfoIndices) {
                    auto& info = infos[j];
                    auto& modelAS = info.modelAs;
                    info.build
                        .setSrcAccelerationStructure(modelAS.blas)
                        .setDstAccelerationStructure(modelAS.blas)
                        .setScratchData(getBufferAddress(scratch_->getBuffer()));
                    cmdbuf.buildAccelerationStructuresKHR(info.build, info.offsets.data(), manager->dispatcher);

                    vk::MemoryBarrier barrier{};
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

            // Reset
            batchSize = 0;
            buildInfoIndices.clear();
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
}

} // namespace wen