#pragma once

#include "ray_tracing/gltf/gltf_scene.hpp"

namespace wen {

struct InstanceAddress {
    vk::DeviceAddress vertexBufferAddress;
    vk::DeviceAddress indexBufferAddress;
};

struct GroupInfo {
    uint32_t offset = 0;
    uint32_t count = 0;
};

class RayTracingInstance {
public:
    RayTracingInstance();
    ~RayTracingInstance();

    void addModels(uint32_t id, std::vector<std::shared_ptr<RayTracingModel>> models, const glm::mat4& matrix);
    void addScene(uint32_t id, std::shared_ptr<RayTracingScene> scene);
    void build(bool allow_update);
    using FunUpdateTransform = std::function<void(const glm::mat4&)>;
    using FunUpdateCallback = std::function<void(uint32_t, FunUpdateTransform)>;
    void update(uint32_t id, FunUpdateCallback callback);

    auto instanceAddressBuffer() { 
        instanceAddressBuffer_ = std::make_shared<StorageBuffer>(
            instanceCount_ * sizeof(InstanceAddress),
            vk::BufferUsageFlagBits::eStorageBuffer,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        );
        auto ptr = static_cast<uint8_t*>(instanceAddressBuffer_->map());
        memcpy(ptr, instanceAddresses_.data(), buffer_->getSize());
        instanceAddressBuffer_->unmap();
        return instanceAddressBuffer_;
    }

    auto primitiveDataBuffer() {
        primitiveDataBuffer_ = std::make_shared<StorageBuffer>(
            instanceCount_ * sizeof(GLTFPrimitive::GLTFPrimitiveData),
            vk::BufferUsageFlagBits::eStorageBuffer,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        );
        auto ptr = static_cast<uint8_t*>(primitiveDataBuffer_->map());
        memcpy(ptr, primitiveDatas_.data(), buffer_->getSize());
        primitiveDataBuffer_->unmap();
        return primitiveDataBuffer_;
    }

private:
    InstanceAddress createInstanceAddress(RayTracingModel& model);
    using FunUpdate = std::function<void(uint32_t, uint32_t, uint32_t)>;
    void singleThreadUpdate(uint32_t id, FunUpdate update);
    void multiThreadUpdate(uint32_t id, FunUpdate update);

public:
    bool allow_update;
    vk::AccelerationStructureKHR tlas;

private:
    std::map<uint32_t, GroupInfo> groups_;
    uint32_t instanceCount_;
    std::vector<vk::AccelerationStructureInstanceKHR> instances_;

    std::vector<InstanceAddress> instanceAddresses_;
    std::shared_ptr<StorageBuffer> instanceAddressBuffer_;
    std::vector<GLTFPrimitive::GLTFPrimitiveData> primitiveDatas_;
    std::shared_ptr<StorageBuffer> primitiveDataBuffer_;

    std::unique_ptr<Buffer> instanceBuffer_;

    std::unique_ptr<StorageBuffer> buffer_;
    std::unique_ptr<StorageBuffer> scratch_;
};

} // namespace wen