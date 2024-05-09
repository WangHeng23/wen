#pragma once

#include "scenes.hpp"

class GLTFScene : public Scene {
public:
    GLTFScene(std::shared_ptr<wen::Interface> interface) : Scene(interface) {}

    void initialize() override;
    void update(float ts) override;
    void render() override;
    void imgui() override;
    void destroy() override;

private:
    std::unique_ptr<wen::Camera> camera_;

    std::shared_ptr<wen::GLTFScene> scene_;

    std::shared_ptr<wen::DescriptorSet> descriptorSet_;
    std::shared_ptr<wen::VertexInput> vertexInput_;
    std::shared_ptr<wen::GraphicsShaderProgram> shaderProgram_;
    std::shared_ptr<wen::GraphicsRenderPipeline> renderPipeline_;

    std::shared_ptr<wen::VertexBuffer> vertexBuffer_;
    std::shared_ptr<wen::IndexBuffer> indexBuffer_;
};