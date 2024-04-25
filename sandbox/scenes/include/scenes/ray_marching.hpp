#pragma once

#include "scenes.hpp"

struct RayMarchingInfo {
    RayMarchingInfo(wen::Interface& interface);

    struct RayMarchingUniform {
        alignas(16) glm::vec2 windowSize;
        alignas(4) int maxSteps;
        alignas(4) float maxDist;
        alignas(4) float epsillonDist;
        alignas(16) glm::vec4 sphere;
        alignas(16) glm::vec3 light;
        alignas(4) float intensity;
    };

    RayMarchingUniform* data;
    std::shared_ptr<wen::UniformBuffer> uniformBuffer;
};

class RayMarching : public Scene {
public:
    RayMarching(std::shared_ptr<wen::Interface> interface) : Scene(interface) {}

    void initialize() override;
    void update(float ts) override;
    void render() override;
    void imgui() override;
    void destroy() override;

private:
    std::unique_ptr<wen::Camera> camera_;
    std::unique_ptr<RayMarchingInfo> info_;
    std::shared_ptr<wen::GraphicsShaderProgram> shaderProgram_;
    std::shared_ptr<wen::GraphicsRenderPipeline> renderPipeline_;
};