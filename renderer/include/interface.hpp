#pragma once

#include "resources/render_pass.hpp"
#include "resources/shader.hpp"
#include "resources/shader_program.hpp"
#include "resources/render_pipeline.hpp"
#include "renderer.hpp"
#include "resources/vertex_input.hpp"
#include "storage/vertex_buffer.hpp"

namespace wen {

class Interface {
public:
    Interface(const std::string& path);
    std::shared_ptr<RenderPass> createRenderPass();
    std::shared_ptr<Renderer> createRenderer(std::shared_ptr<RenderPass> renderPass);
    std::shared_ptr<Shader> createShader(const std::string& filename);
    std::shared_ptr<Shader> compileShader(const std::string& filename, ShaderStage stage);
    std::shared_ptr<GraphicsShaderProgram> createGraphicsShaderProgram();
    std::shared_ptr<GraphicsRenderPipeline> createGraphicsRenderPipeline(std::weak_ptr<Renderer> renderer, std::shared_ptr<GraphicsShaderProgram> shaderProgram, const std::string& subpassName);
    std::shared_ptr<VertexInput> createVertexInput(const std::vector<VertexInputInfo>& infos);
    std::shared_ptr<VertexBuffer> createVertexBuffer(uint32_t size, uint32_t count, vk::BufferUsageFlags additionalUsage = {});

private:
    std::string path_;
    std::string shaderDir_;
};

} // namespace wen