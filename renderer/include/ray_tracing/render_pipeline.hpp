#pragma once

#include "resources/render_pipeline.hpp"
#include "ray_tracing/shader_program.hpp"

namespace wen {

struct RayTracingRenderPipelineOptions {
    uint32_t maxRayRecursionDepth = 1;
};

class RayTracingRenderPipeline : public RenderPipelineTemplate<RayTracingRenderPipeline, RayTracingRenderPipelineOptions> {
    friend class Renderer;

public:
    RayTracingRenderPipeline(std::shared_ptr<RayTracingShaderProgram> shaderProgram);
    ~RayTracingRenderPipeline() override;

    void compile(const RayTracingRenderPipelineOptions& options = {}) override;

private:
    std::shared_ptr<RayTracingShaderProgram> shaderProgram_;
    std::unique_ptr<Buffer> buffer_;
    vk::StridedDeviceAddressRegionKHR raygenRegion_ = {};
    vk::StridedDeviceAddressRegionKHR missRegion_ = {};
    vk::StridedDeviceAddressRegionKHR hitRegion_ = {};
    vk::StridedDeviceAddressRegionKHR callableRegion_ = {};
};

} // namespace wen