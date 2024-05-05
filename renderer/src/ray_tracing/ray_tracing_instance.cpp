#include "ray_tracing/ray_tracing_instance.hpp"
#include "utils/utils.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

namespace wen {

RayTracingInstance::RayTracingInstance() : instanceCount_(0) {}

RayTracingInstance::~RayTracingInstance() {
    instanceBuffer_.reset();
    manager->device->device.destroyAccelerationStructureKHR(tlas, nullptr, manager->dispatcher);
    scratch_.reset();
    buffer_.reset();
}

void RayTracingInstance::addModel(uint32_t id, std::vector<std::shared_ptr<RayTracingModel>> models, const glm::mat4& matrix) {
    for (auto& model : models) {
        auto& blas = model->modelAs.value()->blas;
        instances_.emplace_back()
            // 在着色器中被用来区分不同的实例
            .setInstanceCustomIndex(instanceCount_)
            // 用于在着色器绑定表中找到对应的着色器
            .setInstanceShaderBindingTableRecordOffset(0)
            // 掩码用于在光线追踪时过滤实例
            .setMask(0xff)
            // 禁用三角形剔除
            .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable)
            // 设置加速结构的引用
            .setAccelerationStructureReference(getAccelerationStructureAddress(blas))
            // 用于将实例的加速结构从模型空间变换到世界空间
            .setTransform(convert<vk::TransformMatrixKHR, const glm::mat4&>(matrix));
        instanceAddresses_.push_back(createInstanceAddress(*model));
        instanceCount_++;
        groups_[id].count++;
    }
}

void RayTracingInstance::build(bool allow_update) {
    int count = 0;
    for (auto& [id, group] : groups_) {
        group.offset = count;
        count += group.count;
    }
    for (auto& [id, group] : groups_) {
        WEN_INFO("group id: {0}, offset: {1}, count: {2}", id, group.offset, group.count);
    }

    this->allow_update = allow_update;
    instanceBuffer_ = std::make_unique<Buffer>(
        instanceCount_ * sizeof(vk::AccelerationStructureInstanceKHR),
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    auto* ptr = static_cast<uint8_t*>(instanceBuffer_->map());
    memcpy(ptr, instances_.data(), instanceCount_ * sizeof(vk::AccelerationStructureInstanceKHR));

    // 将之前拷贝上传的实体设备内存进行设置打包
    vk::AccelerationStructureGeometryInstancesDataKHR geometryInstances = {};
    geometryInstances.setData(getBufferAddress(instanceBuffer_->buffer));
    // 我们需要将实体数据放入联合体中并指定该数据为实体数据
    vk::AccelerationStructureGeometryKHR geometry = {};
    geometry.setGeometry(geometryInstances)
            .setGeometryType(vk::GeometryTypeKHR::eInstances);
    
    vk::AccelerationStructureBuildGeometryInfoKHR build = {};
    // 在构建加速结构时，优先考虑光线追踪的速度
    vk::BuildAccelerationStructureFlagsKHR flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    if (allow_update) {
        flags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
    }
    build.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
         .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
         .setFlags(flags)
         .setGeometries(geometry);

    // 获取加速结构大小
    auto size = manager->device->device.getAccelerationStructureBuildSizesKHR(
        vk::AccelerationStructureBuildTypeKHR::eDevice,
        build, instanceCount_, manager->dispatcher
    );

    // 创建顶层加速结构
    vk::AccelerationStructureCreateInfoKHR createInfo = {};
    buffer_ = std::make_unique<StorageBuffer>(
        size.accelerationStructureSize,
        vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );
    createInfo.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
              .setSize(size.accelerationStructureSize)
              .setBuffer(buffer_->getBuffer());
    tlas = manager->device->device.createAccelerationStructureKHR(createInfo, nullptr, manager->dispatcher);

    // 构建顶层加速结构
    auto cmdbuf = manager->commandPool->allocateSingleUse();
    scratch_ = std::make_unique<StorageBuffer>(
        size.buildScratchSize,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );
    build.setDstAccelerationStructure(tlas)
         .setScratchData(getBufferAddress(scratch_->getBuffer()));
    vk::AccelerationStructureBuildRangeInfoKHR range = {};
    range.setPrimitiveCount(instanceCount_)
         .setPrimitiveOffset(0)
         .setFirstVertex(0)
         .setTransformOffset(0);
    cmdbuf.buildAccelerationStructuresKHR(build, &range, manager->dispatcher);
    manager->commandPool->freeSingleUse(cmdbuf);
}

void RayTracingInstance::update(uint32_t id, FunUpdateCallback callback) {
    if (!allow_update) {
        WEN_WARN("you had set allow_update to false, you can't update the instance!")
        return;
    }

    auto update = [=](uint32_t index, uint32_t begin, uint32_t end) {
        auto* asInstancePtr = static_cast<vk::AccelerationStructureInstanceKHR*>(instanceBuffer_->data);
        asInstancePtr += begin;
        FunUpdateTransform updateTransform = [&](const glm::mat4& transform) {
            asInstancePtr->setTransform(convert<vk::TransformMatrixKHR, const glm::mat4&>(transform));
        };
        while (begin < end) {
            callback(index, updateTransform);
            asInstancePtr++;
            index++;
            begin++;
        }
    };

#define MT 1
#if MT
    multiThreadUpdate(id, update);
#else
    singleThreadUpdate(id, update);
#endif

    vk::AccelerationStructureGeometryInstancesDataKHR geometryInstances = {};
    geometryInstances.setData(getBufferAddress(instanceBuffer_->buffer));
    vk::AccelerationStructureGeometryKHR geometry = {};
    geometry.setGeometry(geometryInstances)
            .setGeometryType(vk::GeometryTypeKHR::eInstances);

    vk::AccelerationStructureBuildGeometryInfoKHR build = {};
    build.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
         .setMode(vk::BuildAccelerationStructureModeKHR::eUpdate)
         .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate)
         .setGeometries(geometry);
    
    auto size = manager->device->device.getAccelerationStructureBuildSizesKHR(
        vk::AccelerationStructureBuildTypeKHR::eDevice,
        build, instanceCount_, manager->dispatcher
    );

    auto cmdbuf = manager->commandPool->allocateSingleUse();
    if (scratch_->getSize() < size.updateScratchSize) {
        scratch_.reset();
        scratch_ = std::make_unique<StorageBuffer>(
            size.updateScratchSize,
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            VMA_MEMORY_USAGE_GPU_ONLY,
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
        );
    }
    build.setSrcAccelerationStructure(tlas)
         .setDstAccelerationStructure(tlas)
         .setScratchData(getBufferAddress(scratch_->getBuffer()));
    vk::AccelerationStructureBuildRangeInfoKHR range = {};
    range.setPrimitiveCount(instanceCount_)
         .setPrimitiveOffset(0)
         .setFirstVertex(0)
         .setTransformOffset(0);
    cmdbuf.buildAccelerationStructuresKHR(build, &range, manager->dispatcher);
    manager->commandPool->freeSingleUse(cmdbuf);
}

RayTracingInstanceAddress RayTracingInstance::createInstanceAddress(RayTracingModel& model) {
    switch (model.getType()) {
        case RayTracingModel::ModelType::eNormalModel:
            return {
                getBufferAddress(dynamic_cast<Model&>(model).rayTracingVertexBuffer->buffer),
                getBufferAddress(dynamic_cast<Model&>(model).rayTracingIndexBuffer->buffer),
            };
    }
}

void RayTracingInstance::singleThreadUpdate(uint32_t id, FunUpdate update) {
    auto groupInfo = groups_.at(id);
    uint32_t begin = groupInfo.offset;
    uint32_t end = begin + groupInfo.count;
    
    uint32_t index = 0;
    update(index, begin, end);
}

void RayTracingInstance::multiThreadUpdate(uint32_t id, FunUpdate update) {
    auto groupInfo = groups_.at(id);
    uint32_t begin = groupInfo.offset;
    uint32_t end = begin + groupInfo.count;
    
    uint32_t index = 0;
    std::vector<std::thread> threads;
    while (begin < end) {
        // 每批次更新1000个实例
        uint32_t batchEnd = std::min(begin + 1000, end);
        // 不能传引用传参，避免数据竞争
        threads.emplace_back([=]() {
            update(index, begin, batchEnd);
        });
        index += 1000;
        begin = batchEnd;
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

} // namespace wen