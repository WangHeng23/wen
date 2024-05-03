#include "ray_tracing/ray_tracing_model.hpp"
#include "manager.hpp"

namespace wen {

ModelAccelerationStructure::~ModelAccelerationStructure() {
    manager->device->device.destroyAccelerationStructureKHR(blas, nullptr, manager->dispatcher);
    buffer.reset();
}

RayTracingModel::~RayTracingModel() {
    if (modelAs.has_value()) {
        modelAs.value().reset();
        modelAs->reset();
    }
}

} // namespace wen