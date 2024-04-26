#pragma once

#include "scenes.hpp"

// 一个模型的信息
struct ModelInfo {
    struct InnerInfo {
        glm::vec3 offset;
        float scale;
    };

    std::shared_ptr<wen::Model> model;
    std::shared_ptr<wen::VertexBuffer> vertexBuffer;
    std::vector<InnerInfo> innerInfos;
    std::map<std::string, bool> meshNameVisible;
};

class ModelManager : public Scene {
public:
    ModelManager(std::shared_ptr<wen::Interface> interface) : Scene(interface) {}

    void initialize() override;
    void update(float ts) override;
    void render() override;
    void imgui() override;
    void destroy() override;

private:
    std::unique_ptr<wen::Camera> camera_;
    std::shared_ptr<wen::GraphicsShaderProgram> shaderProgram_;
    std::shared_ptr<wen::GraphicsRenderPipeline> renderPipeline_;
    std::shared_ptr<wen::VertexBuffer> vertexBuffer_;
    std::shared_ptr<wen::IndexBuffer> indexBuffer_;
    // 模型对应的文件名和模型信息
    std::map<std::string, ModelInfo> models_;
    // 实例名 (模型文件名 实例索引)
    std::map<std::string, std::pair<std::string, uint32_t>> querys;
};