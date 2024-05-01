#include "ray_tracing/ray_tracing_instance.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

RayTracingInstance::RayTracingInstance(bool allow_update)
    : allow_update_(allow_update), instanceCount_(0) {}

RayTracingInstance::~RayTracingInstance() {
    instanceBuffer_.reset();
    manager->device->device.destroyAccelerationStructureKHR(tlas_, nullptr, manager->dispatcher);
    scratch_.reset();
    buffer_.reset();
}

void RayTracingInstance::addInstance(std::vector<std::shared_ptr<RayTracingModel>> models, const glm::mat4& matrix) {
    std::vector<vk::AccelerationStructureInstanceKHR> instances;
    std::vector<RayTracingInstanceAddress> instanceAddresses;
    for (auto& model : models) {
        auto& blas = model->modelAs.value()->blas;
        instances.emplace_back()
            .setInstanceCustomIndex(instanceAddresses.size())
            .setInstanceShaderBindingTableRecordOffset(0)
            .setMask(0xff)
            .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable)
            .setAccelerationStructureReference(getAccelerationStructureAddress(blas))
            .setTransform(convert<vk::TransformMatrixKHR, const glm::mat4&>(matrix));
        instanceAddresses.push_back(createInstanceAddress(*model));
    }

    instanceBuffer_ = std::make_unique<Buffer>(
        instances.size() * sizeof(vk::AccelerationStructureInstanceKHR),
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    auto* ptr = static_cast<uint8_t*>(instanceBuffer_->map());
    memcpy(ptr, instances.data(), instances.size() * sizeof(vk::AccelerationStructureInstanceKHR));
    instanceBuffer_->unmap();

    instanceAddressBuffer_ = std::make_unique<Buffer>(
        instanceAddresses.size() * sizeof(RayTracingInstanceAddress),
        vk::BufferUsageFlagBits::eStorageBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    );
    ptr = static_cast<uint8_t*>(instanceAddressBuffer_->map());
    memcpy(ptr, instanceAddresses.data(), instanceAddresses.size() * sizeof(RayTracingInstanceAddress));
    instanceAddressBuffer_->unmap();
}

void RayTracingInstance::build() {
    vk::AccelerationStructureBuildRangeInfoKHR range = {};
    range.setPrimitiveCount(1)
         .setPrimitiveOffset(0)
         .setFirstVertex(0)
         .setTransformOffset(0);
    
    vk::AccelerationStructureGeometryInstancesDataKHR geometryInstances = {};
    geometryInstances.setData(getBufferAddress(instanceBuffer_->getBuffer()));
    vk::AccelerationStructureGeometryKHR geometry = {};
    geometry.setGeometry(geometryInstances)
            .setGeometryType(vk::GeometryTypeKHR::eInstances);
    
    vk::AccelerationStructureBuildGeometryInfoKHR build = {};
    vk::BuildAccelerationStructureFlagsKHR flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    if (allow_update_) {
        flags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
    }
    build.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
         .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
         .setFlags(flags)
         .setGeometries(geometry);
        
    auto size = manager->device->device.getAccelerationStructureBuildSizesKHR(
        vk::AccelerationStructureBuildTypeKHR::eDevice,
        build, 1, manager->dispatcher
    );
    scratch_ = std::make_unique<Buffer>(
        size.buildScratchSize,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );
    buffer_ = std::make_shared<Buffer>(
        size.accelerationStructureSize,
        vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );

    vk::AccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
              .setSize(size.accelerationStructureSize)
              .setBuffer(buffer_->getBuffer());
    tlas_ = manager->device->device.createAccelerationStructureKHR(createInfo, nullptr, manager->dispatcher);

    auto cmdBuffer = manager->commandPool->allocateSingleUse();
    build.setDstAccelerationStructure(tlas_)
         .setScratchData(getBufferAddress(scratch_->getBuffer()));
    cmdBuffer.buildAccelerationStructuresKHR(build, &range, manager->dispatcher);
    manager->commandPool->freeSingleUse(cmdBuffer);

    if (!allow_update_) {
        scratch_.reset();
        instanceBuffer_.reset();
    }
}

RayTracingInstanceAddress RayTracingInstance::createInstanceAddress(RayTracingModel& model) {
    switch (model.getType()) {
        case RayTracingModel::ModelType::eNormalModel:
            return {
                getBufferAddress(dynamic_cast<Model&>(model).vertexBuffer->getBuffer()),
                getBufferAddress(dynamic_cast<Model&>(model).indexBuffer->getBuffer()),
            };
    }
}

} // namespace wen