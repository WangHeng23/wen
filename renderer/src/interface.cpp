#include "interface.hpp"
#include "utils/utils.hpp"
#include "core/logger.hpp"

namespace wen {

Interface::Interface(const std::string& path) : path_(path) {
    shaderDir_ = path_ + "/shaders/";
    textureDir_ = path_ + "/textures/";
    modelDir_ = path_ + "/models/";
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

std::shared_ptr<DescriptorSet> Interface::createDescriptorSet() {
    return std::make_shared<DescriptorSet>();
}

std::shared_ptr<UniformBuffer> Interface::createUniformBuffer(uint64_t size, bool inFlight) {
    return std::make_shared<UniformBuffer>(size, inFlight);
}

std::shared_ptr<Texture> Interface::createTexture(const std::string& filename, uint32_t mipLevels) {
    auto pos = filename.find_last_of('.') + 1;
    auto filetype = filename.substr(pos, filename.size() - pos);
    std::string filepath = textureDir_ + filename;
    if (filetype == "png" || filetype == "jpg") {
        return std::make_shared<ImageTexture>(filepath, mipLevels);
    }
    WEN_ERROR("Unsupported texture file type: {}", filetype)
    return nullptr;
}

std::shared_ptr<Texture> Interface::createTexture(const uint8_t* data, uint32_t width, uint32_t height, uint32_t mipLevels) {
    return std::make_shared<DataTexture>(data, width, height, mipLevels);
}

std::shared_ptr<Sampler> Interface::createSampler(const SamplerInfos& infos) {
    return std::make_shared<Sampler>(infos);
}

std::shared_ptr<PushConstants> Interface::createPushConstants(ShaderStages stages, const std::vector<PushConstantInfo>& infos) {
    return std::make_shared<PushConstants>(stages, infos);
}

std::shared_ptr<Model> Interface::loadModel(const std::string& filename) {
    return std::make_shared<Model>(modelDir_ + filename);
}

std::shared_ptr<StorageImage> Interface::createStorageImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags additionalUsage) {
    return std::make_shared<StorageImage>(width, height, format, additionalUsage);
}

std::shared_ptr<RayTracingShaderProgram> Interface::createRayTracingShaderProgram() {
    return std::make_shared<RayTracingShaderProgram>();
}

std::shared_ptr<RayTracingRenderPipeline> Interface::createRayTracingRenderPipeline(std::shared_ptr<RayTracingShaderProgram> shaderProgram) {
    return std::make_shared<RayTracingRenderPipeline>(shaderProgram);
}

std::shared_ptr<AccelerationStructure> Interface::createAccelerationStructure() {
    return std::make_shared<AccelerationStructure>();
}

std::shared_ptr<RayTracingInstance> Interface::createRayTracingInstance(bool allow_update) {
    return std::make_shared<RayTracingInstance>(allow_update);
}

} // namespace wen