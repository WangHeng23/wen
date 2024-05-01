#pragma once

#include "resources/buffer.hpp"
#include "resources/model.hpp"

namespace wen {

struct AccelerationStructureInfo {
    AccelerationStructureInfo(ModelAccelerationStructure& modelAs) : modelAs(modelAs) {}

    ModelAccelerationStructure& modelAs;
    std::vector<vk::AccelerationStructureGeometryDataKHR> geometryDatas = {};
    std::vector<vk::AccelerationStructureGeometryKHR> geometries = {};
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> offsets = {};
    vk::AccelerationStructureBuildGeometryInfoKHR build = {};
    vk::AccelerationStructureBuildSizesInfoKHR size = {};
    vk::AccelerationStructureKHR as = {};
    std::unique_ptr<Buffer> buffer = {};
};

class AccelerationStructure {
public:
    AccelerationStructure() = default;
    ~AccelerationStructure();
    void addModel(std::shared_ptr<RayTracingModel> model);
    void build(bool is_update, bool allow_update);

private:
    std::unique_ptr<Buffer> staging_ = {};
    uint64_t currentStagingSize_ = 0;
    std::unique_ptr<Buffer> scratch_ = {};
    uint64_t currentScratchSize_ = 0;
    std::vector<std::shared_ptr<Model>> models_;
};

} // namespace wen