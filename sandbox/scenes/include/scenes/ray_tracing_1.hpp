#pragma once

#include "scenes.hpp"

class RayTracing : public Scene {
public:
    struct Info {
        alignas(16) glm::vec2 windowSize;
        alignas(16) glm::vec3 clearColor;
    };

public:
    RayTracing(std::shared_ptr<wen::Interface> interface) : Scene(interface) {
        isEnableRayTracing = false;
    }

    void initialize() override;
    void update(float ts) override;
    void render() override;
    void imgui() override;
    void destroy() override;

    void createAccelerationStructure();

private:
    std::unique_ptr<wen::Camera> camera_;

    std::shared_ptr<wen::RayTracingInstance> instance_;
    std::shared_ptr<wen::VertexBuffer> vertexBuffer_;
    std::shared_ptr<wen::IndexBuffer> indexBuffer_;
    std::shared_ptr<wen::Model> model1_;
    std::shared_ptr<wen::Model> model2_;
    std::shared_ptr<wen::Model> model3_;
    std::vector<std::tuple<glm::vec3, glm::vec3, float, float>> transformInfos;

    Info* info_;
    std::shared_ptr<wen::UniformBuffer> infoUniform_;
    glm::vec3 pointLightPosition_;
    std::shared_ptr<wen::PushConstants> pushConstants_;

    std::shared_ptr<wen::DescriptorSet> shaderDescriptorSet_;

    // ray tracing
    std::shared_ptr<wen::DescriptorSet> rayTracingDescriptorSet_;
    std::shared_ptr<wen::RayTracingShaderProgram> rayTracingShaderProgram_;
    std::shared_ptr<wen::RayTracingRenderPipeline> rayTracingRenderPipeline_;
    std::shared_ptr<wen::DescriptorSet> imageDescriptorSet_;
    std::shared_ptr<wen::GraphicsShaderProgram> graphicsShaderProgram_;
    std::shared_ptr<wen::GraphicsRenderPipeline> graphicsRenderPipeline_;
    std::shared_ptr<wen::StorageImage> image_;
    std::shared_ptr<wen::Sampler> sampler_;

    // no ray tracing
    std::shared_ptr<wen::GraphicsShaderProgram> shaderProgram_;
    std::shared_ptr<wen::GraphicsRenderPipeline> renderPipeline_;
};