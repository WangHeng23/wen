#pragma once

#include "scenes.hpp"

class ShaderToyInput {
public:
    ShaderToyInput(wen::Interface& interface);

    struct ShadertoyInputUniform {
        alignas(16) glm::vec3 iResolution;
        alignas(4) float iTime;
        alignas(4) float iTimeDelta;
        alignas(4) float iFrameRate;
        alignas(4) float iFrame;
        alignas(16) glm::vec4 iMouse;
        alignas(16) glm::vec4 iDate;
    };

    ShadertoyInputUniform* data;
    std::shared_ptr<wen::UniformBuffer> uniformBuffer;
};

class ShaderToy : public Scene {
public:
    ShaderToy(std::shared_ptr<wen::Interface> interface) : Scene(interface) {}

    void initialize() override;
    void update(float ts) override;
    void render() override;
    void imgui() override;
    void destroy() override;

private:
    float time_ = 0.0f;
    std::unique_ptr<ShaderToyInput> input_;
    std::shared_ptr<wen::GraphicsShaderProgram> shaderProgram_;
    std::shared_ptr<wen::GraphicsRenderPipeline> renderPipeline_;
    std::shared_ptr<wen::PushConstants> pushConstants_;
    std::shared_ptr<wen::VertexBuffer> vertexBuffer_;
    std::shared_ptr<wen::IndexBuffer> indexBuffer_;
};