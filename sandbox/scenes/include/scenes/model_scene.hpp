#pragma once

#include "scenes.hpp"

class ModelScene : public Scene {
public:
    struct Light {
        Light(std::shared_ptr<wen::Interface> interface);

        struct PointLight {
            alignas(16) glm::vec3 position;
            alignas(16) glm::vec3 color;
            alignas(4) float intensity;
        };

        struct LightUniform {
            alignas(16) PointLight lights[8];
            alignas(4) uint32_t lightCount;
        };

        LightUniform* data;
        std::shared_ptr<wen::UniformBuffer> uniformBuffer;
    };

public:
    ModelScene(std::shared_ptr<wen::Interface> interface) : Scene(interface) {}

    void initialize() override;
    void update(float ts) override;
    void render() override;
    void imgui() override;
    void destroy() override;

private:
    int n;
    std::vector<glm::vec3> offsets;
    glm::vec3 clearColor = {0.2f, 0.3f, 0.3f};
    std::unique_ptr<wen::Camera> camera;
    std::unique_ptr<Light> light;
    std::shared_ptr<wen::GraphicsShaderProgram> shaderProgram;
    std::shared_ptr<wen::GraphicsRenderPipeline> renderPipeline;
    std::shared_ptr<wen::Model> model;
    std::shared_ptr<wen::VertexBuffer> vertexBuffer;
    std::shared_ptr<wen::IndexBuffer> indexBuffer;
    std::shared_ptr<wen::VertexBuffer> offsetsBuffer;
};