#include "ray_tracing/shader_program.hpp"

namespace wen {

void RayTracingShaderProgram::setRaygenShader(std::shared_ptr<Shader> shader, const std::string& entry) {
    raygenShader_.shader = std::dynamic_pointer_cast<Shader>(shader);
    raygenShader_.entry = entry;
}

void RayTracingShaderProgram::setMissShader(std::shared_ptr<Shader> shader, const std::string& entry) {
    missShaders_.emplace_back(shader, entry);
} 

void RayTracingShaderProgram::setHitGroup(HitGroup hitGroup) {
    hitGroups_.emplace_back(hitGroup);
    hitShaderCount_++;
    if (hitGroup.intersectionShader.has_value()) {
        hitShaderCount_++;
    }
}

RayTracingShaderProgram::~RayTracingShaderProgram() {
    raygenShader_.shader.reset();
    missShaders_.clear();
    hitGroups_.clear();
    hitShaderCount_ = 0;
}

} // namespace wen