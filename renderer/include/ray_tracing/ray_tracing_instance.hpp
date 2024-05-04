#pragma once

#include "ray_tracing/ray_tracing_model.hpp"
#include "storage/storage_buffer.hpp"
#include <glm/glm.hpp>

namespace wen {

struct RayTracingInstanceAddress {
    vk::DeviceAddress vertexBufferAddress;
    vk::DeviceAddress indexBufferAddress;
};

class RayTracingInstance {
public:
    RayTracingInstance();
    ~RayTracingInstance();

    void addModel(std::vector<std::shared_ptr<RayTracingModel>> models, const glm::mat4& matrix);
    void build(bool allow_update);
    using FunUpdateTransform = std::function<void(const glm::mat4&)>;
    using FunUpdateCallback = std::function<void(uint32_t, FunUpdateTransform)>;
    void update(FunUpdateCallback callback);

    auto instanceAddressBuffer() { return instanceAddressBuffer_; }
    auto tlas() { return tlas_; }

private:
    RayTracingInstanceAddress createInstanceAddress(RayTracingModel& model);

private:
    bool allow_update_;
    uint32_t instanceCount_;

    std::vector<vk::AccelerationStructureInstanceKHR> instances;
    std::vector<RayTracingInstanceAddress> instanceAddresses;

    std::unique_ptr<Buffer> instanceBuffer_;
    std::shared_ptr<StorageBuffer> instanceAddressBuffer_;

    vk::AccelerationStructureKHR tlas_;
    std::unique_ptr<StorageBuffer> buffer_;
    std::unique_ptr<StorageBuffer> scratch_;
};

} // namespace wen