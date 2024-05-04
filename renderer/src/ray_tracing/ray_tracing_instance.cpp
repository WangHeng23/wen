#include "ray_tracing/ray_tracing_instance.hpp"
#include "utils/utils.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

namespace wen {

RayTracingInstance::RayTracingInstance() : instanceCount_(0) {}

RayTracingInstance::~RayTracingInstance() {
    instanceBuffer_.reset();
    manager->device->device.destroyAccelerationStructureKHR(tlas_, nullptr, manager->dispatcher);
    scratch_.reset();
    buffer_.reset();
}

void RayTracingInstance::addModel(std::vector<std::shared_ptr<RayTracingModel>> models, const glm::mat4& matrix) {
    for (auto& model : models) {
        auto& blas = model->modelAs.value()->blas;
        instances.emplace_back()
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
        instanceAddresses.push_back(createInstanceAddress(*model));
        instanceCount_++;
    }
}

void RayTracingInstance::build(bool allow_update) {
    instanceBuffer_ = std::make_unique<Buffer>(
        instanceCount_ * sizeof(vk::AccelerationStructureInstanceKHR),
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    auto* ptr = static_cast<uint8_t*>(instanceBuffer_->map());
    memcpy(ptr, instances.data(), instanceCount_ * sizeof(vk::AccelerationStructureInstanceKHR));
    instanceBuffer_->unmap();

    instanceAddressBuffer_ = std::make_unique<StorageBuffer>(
        instanceCount_ * sizeof(RayTracingInstanceAddress),
        vk::BufferUsageFlagBits::eStorageBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    ptr = static_cast<uint8_t*>(instanceAddressBuffer_->map());
    memcpy(ptr, instanceAddresses.data(), instanceCount_ * sizeof(RayTracingInstanceAddress));
    instanceAddressBuffer_->unmap();

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
    tlas_ = manager->device->device.createAccelerationStructureKHR(createInfo, nullptr, manager->dispatcher);

    // 构建顶层加速结构
    auto cmdbuf = manager->commandPool->allocateSingleUse();
    scratch_ = std::make_unique<StorageBuffer>(
        size.buildScratchSize,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );
    build.setDstAccelerationStructure(tlas_)
         .setScratchData(getBufferAddress(scratch_->getBuffer()));
    vk::AccelerationStructureBuildRangeInfoKHR range = {};
    range.setPrimitiveCount(instanceCount_)
         .setPrimitiveOffset(0)
         .setFirstVertex(0)
         .setTransformOffset(0);
    cmdbuf.buildAccelerationStructuresKHR(build, &range, manager->dispatcher);
    manager->commandPool->freeSingleUse(cmdbuf);

    if (!allow_update_) {
        scratch_.reset();
        instanceBuffer_.reset();
    }
}

void RayTracingInstance::update(FunUpdateCallback callback) {
    if (!allow_update_) {
        WEN_WARN("you had set allow_update to false, you can't call this function")
        return;
    }

    auto update = [&](uint32_t begin, uint32_t end) {
        auto currentInstance = instances.begin();
        for (uint32_t i = 0; i < begin; i++) {
            currentInstance++;
        }
        FunUpdateTransform updateTransform = [&](const glm::mat4& transform) {
            currentInstance->setTransform(convert<vk::TransformMatrixKHR, const glm::mat4&>(transform));
        };
        while (begin < end) {
            callback(begin, updateTransform);
            currentInstance++;
            begin++;
        }
    };
    uint32_t index = 0;
    std::vector<std::thread> threads;
    while (index < instanceCount_) {
        uint32_t end = std::min(index + 100, instanceCount_);
        threads.emplace_back([=]() {
            update(index, end);
        });
        index = end;
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto* ptr = static_cast<uint8_t*>(instanceBuffer_->map());
    memcpy(ptr, instances.data(), instanceCount_ * sizeof(vk::AccelerationStructureInstanceKHR));
    instanceBuffer_->unmap();

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
    build.setSrcAccelerationStructure(tlas_)
         .setDstAccelerationStructure(tlas_)
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

} // namespace wen