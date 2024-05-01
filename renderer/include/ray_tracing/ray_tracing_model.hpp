#pragma once

#include "resources/buffer.hpp"

namespace wen {

struct ModelAccelerationStructure {
    ModelAccelerationStructure() = default;
    ~ModelAccelerationStructure();
    vk::AccelerationStructureKHR blas;
    std::unique_ptr<Buffer> buffer;
};

class RayTracingModel {
public:
    enum class ModelType {
        eNormalModel,
    };

public:
    virtual ModelType getType() const = 0;
    virtual ~RayTracingModel() = default;

    std::optional<std::unique_ptr<ModelAccelerationStructure>> modelAs;
};

} // namespace wen