#include "interface.hpp"
#include "utils/utils.hpp"
#include "core/logger.hpp"

namespace wen {

Interface::Interface(const std::string& path) : path_(path) {
    shaderDir_ = path_ + "/shaders/";
}

std::shared_ptr<RenderPass> Interface::createRenderPass() {
    return std::make_shared<RenderPass>();
}

std::shared_ptr<Renderer> Interface::createRenderer(std::shared_ptr<RenderPass> renderPass) {
    return std::make_shared<Renderer>(renderPass);
}

std::shared_ptr<Shader> Interface::createShader(const std::string& filename) {
    auto code = readFile(shaderDir_ + filename);
    return std::make_shared<Shader>(code);
}

std::shared_ptr<Shader> Interface::compileShader(const std::string& filename, ShaderStage stage) {
    std::string filepath = shaderDir_ + filename;
    auto code = readFile(filepath);
    if (code.empty()) {
        WEN_ERROR("Shader file is empty: {}", filepath)
        return nullptr;
    }
    return std::make_shared<Shader>(filepath, std::string(code.begin(), code.end()), stage);
}

std::shared_ptr<GraphicsShaderProgram> Interface::createGraphicsShaderProgram() {
    return std::make_shared<GraphicsShaderProgram>();
}

std::shared_ptr<GraphicsRenderPipeline> Interface::createGraphicsRenderPipeline(std::weak_ptr<Renderer> renderer, std::shared_ptr<GraphicsShaderProgram> shaderProgram, const std::string& subpassName) {
    return std::make_shared<GraphicsRenderPipeline>(renderer, shaderProgram, subpassName);
}

std::shared_ptr<VertexInput> Interface::createVertexInput(const std::vector<VertexInputInfo>& infos) {
    return std::make_shared<VertexInput>(infos);
}

std::shared_ptr<VertexBuffer> Interface::createVertexBuffer(uint32_t size, uint32_t count, vk::BufferUsageFlags additionalUsage) {
    return std::make_shared<VertexBuffer>(size, count, additionalUsage);
}

std::shared_ptr<IndexBuffer> Interface::createIndexBuffer(IndexType type, uint32_t count, vk::BufferUsageFlags additionalUsage) {
    return std::make_shared<IndexBuffer>(type, count, additionalUsage);
}

std::shared_ptr<ImGuiLayer> Interface::createImGuiLayer(std::shared_ptr<Renderer>& renderer) {
    return std::make_shared<ImGuiLayer>(renderer);
}

} // namespace wen