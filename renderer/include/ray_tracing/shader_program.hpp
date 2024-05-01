#pragma once

#include "resources/shader_program.hpp"

namespace wen {

class RayTracingShaderProgram : public ShaderProgram {
    friend class RayTracingRenderPipeline;

public:
    struct HitGroup {
        ShaderStage closestHitShader;
        std::optional<ShaderStage> intersectionShader;
    };

public:
    RayTracingShaderProgram() : hitShaderCount_(0) {}
    ~RayTracingShaderProgram() override;

public:
    void setRaygenShader(std::shared_ptr<Shader> shader, const std::string& entry = "main");
    void setMissShader(std::shared_ptr<Shader> shader, const std::string& entry = "main");
    void setHitGroup(HitGroup hitGroup);

private:
    ShaderStage raygenShader_;
    std::vector<ShaderStage> missShaders_;
    std::vector<HitGroup> hitGroups_;
    uint32_t hitShaderCount_;
};

template <>
struct ShaderProgramBindPoint<RayTracingShaderProgram> {
    const static vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eRayTracingKHR;
};

} // namespace wen