#pragma once

#include "storage/storage_buffer.hpp"
#include "resources/model.hpp"

namespace wen {

struct AccelerationStructureInfo {
    AccelerationStructureInfo(ModelAccelerationStructure& modelAs) : modelAs(modelAs) {}

    ModelAccelerationStructure& modelAs;
    std::unique_ptr<StorageBuffer> buffer = {};
    vk::AccelerationStructureKHR as = {};
    std::vector<vk::AccelerationStructureGeometryDataKHR> geometryDatas = {};
    std::vector<vk::AccelerationStructureGeometryKHR> geometries = {};
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> offsets = {};
    vk::AccelerationStructureBuildGeometryInfoKHR build = {};
    vk::AccelerationStructureBuildSizesInfoKHR size = {};
};

class AccelerationStructure {
public:
    AccelerationStructure() = default;
    ~AccelerationStructure();
    void addModel(std::shared_ptr<RayTracingModel> model);
    void build(bool is_update, bool allow_update);

private:
    std::unique_ptr<StorageBuffer> staging_ = {};
    uint64_t currentStagingSize_ = 0;
    std::unique_ptr<StorageBuffer> scratch_ = {};
    uint64_t currentScratchSize_ = 0;
    std::vector<std::shared_ptr<Model>> models_ = {};
};

} // namespace wen