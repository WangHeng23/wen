#pragma once

#include "ray_tracing/ray_tracing_model.hpp"
#include <glm/glm.hpp>

namespace wen {

struct RayTracingInstanceAddress {
    vk::DeviceAddress vertexBufferAddress;
    vk::DeviceAddress indexBufferAddress;
};

class RayTracingInstance {
public:
    RayTracingInstance(bool allow_update);
    ~RayTracingInstance();
    void addInstance(std::vector<std::shared_ptr<RayTracingModel>> models, const glm::mat4& matrix);
    void build();

    auto instanceAddressBuffer() { return instanceAddressBuffer_; }
    auto tlas() { return tlas_; }

private:
    RayTracingInstanceAddress createInstanceAddress(RayTracingModel& model);

private:
    bool allow_update_;

    uint32_t instanceCount_;
    std::unique_ptr<Buffer> instanceBuffer_;
    std::shared_ptr<Buffer> instanceAddressBuffer_;

    vk::AccelerationStructureKHR tlas_;
    std::unique_ptr<Buffer> scratch_;
    std::shared_ptr<Buffer> buffer_;
};

} // namespace wen