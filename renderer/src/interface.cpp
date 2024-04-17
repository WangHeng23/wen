#include "interface.hpp"
#include "utils/utils.hpp"

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

std::shared_ptr<GraphicsShaderProgram> Interface::createGraphicsShaderProgram() {
    return std::make_shared<GraphicsShaderProgram>();
}

std::shared_ptr<GraphicsRenderPipeline> Interface::createGraphicsRenderPipeline(std::weak_ptr<Renderer> renderer, std::shared_ptr<GraphicsShaderProgram> shaderProgram, const std::string& subpassName) {
    return std::make_shared<GraphicsRenderPipeline>(renderer, shaderProgram, subpassName);
}

} // namespace wen