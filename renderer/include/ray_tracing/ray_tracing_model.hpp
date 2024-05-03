#pragma once

#include "storage/storage_buffer.hpp"

namespace wen {

struct ModelAccelerationStructure {
    ModelAccelerationStructure() = default;
    ~ModelAccelerationStructure();
    std::unique_ptr<StorageBuffer> buffer;
    vk::AccelerationStructureKHR blas;
};

class RayTracingModel {
public:
    enum class ModelType {
        eNormalModel,
    };

public:
    virtual ModelType getType() const = 0;
    virtual ~RayTracingModel();

    std::optional<std::unique_ptr<ModelAccelerationStructure>> modelAs;
};

} // namespace wen