#pragma once

#include "ray_tracing/ray_tracing_model.hpp"
#include "storage/storage_buffer.hpp"
#include <glm/glm.hpp>

namespace wen {

struct RayTracingInstanceAddress {
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

    void addModel(uint32_t id, std::vector<std::shared_ptr<RayTracingModel>> models, const glm::mat4& matrix);
    void build(bool allow_update);
    using FunUpdateTransform = std::function<void(const glm::mat4&)>;
    using FunUpdateCallback = std::function<void(uint32_t, FunUpdateTransform)>;
    void update(uint32_t id, FunUpdateCallback callback);

    auto instanceAddressBuffer() { 
        instanceAddressBuffer_ = std::make_shared<StorageBuffer>(
            instanceCount_ * sizeof(RayTracingInstanceAddress),
            vk::BufferUsageFlagBits::eStorageBuffer,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        );
        auto ptr = static_cast<uint8_t*>(instanceAddressBuffer_->map());
        memcpy(ptr, instanceAddresses_.data(), instanceCount_ * sizeof(RayTracingInstanceAddress));
        instanceAddressBuffer_->unmap();
        return instanceAddressBuffer_;
    }

private:
    RayTracingInstanceAddress createInstanceAddress(RayTracingModel& model);
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

    std::vector<RayTracingInstanceAddress> instanceAddresses_;
    std::shared_ptr<StorageBuffer> instanceAddressBuffer_;

    std::unique_ptr<Buffer> instanceBuffer_;

    std::unique_ptr<StorageBuffer> buffer_;
    std::unique_ptr<StorageBuffer> scratch_;
};

} // namespace wen