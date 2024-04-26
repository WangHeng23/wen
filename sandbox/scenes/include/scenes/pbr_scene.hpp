#pragma once

#include "scenes.hpp"

struct PBRMaterial {
    PBRMaterial(wen::Interface& interface);

    struct MaterialUniform {
        alignas(16) glm::vec3 color;
        alignas(4) float roughness;
        alignas(4) float metallic;
        alignas(4) float reflectance;
    };

    MaterialUniform* data;
    std::shared_ptr<wen::UniformBuffer> uniformBuffer;
};

class PBRScene : public Scene {
public:
    struct Light {
        Light(wen::Interface& interface);

        struct PointLight {
            alignas(16) glm::vec3 position;
            alignas(16) glm::vec3 color;
        };

        struct LightUniform {
            alignas(16) glm::vec3 direction;
            alignas(16) glm::vec3 color;
            alignas(16) PointLight pointLights[3];
        };

        LightUniform* data;
        std::shared_ptr<wen::UniformBuffer> uniformBuffer;
    };

public:
    PBRScene(std::shared_ptr<wen::Interface> interface) : Scene(interface) {}

    void initialize() override;
    void update(float ts) override;
    void render() override;
    void imgui() override;
    void destroy() override;

private:
    std::unique_ptr<wen::Camera> camera_;
    std::unique_ptr<Light> light_;
    std::unique_ptr<PBRMaterial> material_;
    std::shared_ptr<wen::GraphicsShaderProgram> shaderProgram_;
    std::shared_ptr<wen::GraphicsRenderPipeline> renderPipeline_;
    std::shared_ptr<wen::Model> model_;
    std::shared_ptr<wen::VertexBuffer> vertexBuffer_;
    std::shared_ptr<wen::IndexBuffer> indexBuffer_;
};