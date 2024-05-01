#pragma once

#include "resources/render_pass.hpp"
#include "resources/shader.hpp"
#include "resources/shader_program.hpp"
#include "resources/render_pipeline.hpp"
#include "renderer.hpp"
#include "resources/vertex_input.hpp"
#include "storage/vertex_buffer.hpp"
#include "storage/index_buffer.hpp"
#include "imgui/imgui.hpp"
#include "storage/descriptor_set.hpp"
#include "storage/uniform_buffer.hpp"
#include "storage/image_texture.hpp"
#include "resources/sampler.hpp"
#include "storage/push_constants.hpp"
#include "resources/model.hpp"
#include "storage/storage_image.hpp"
#include "ray_tracing/shader_program.hpp"
#include "ray_tracing/render_pipeline.hpp"
#include "ray_tracing/acceleration_structure.hpp"
#include "ray_tracing/ray_tracing_instance.hpp"

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
    std::shared_ptr<IndexBuffer> createIndexBuffer(IndexType type, uint32_t count, vk::BufferUsageFlags additionalUsage = {});
    std::shared_ptr<ImGuiLayer> createImGuiLayer(std::shared_ptr<Renderer>& renderer);
    std::shared_ptr<DescriptorSet> createDescriptorSet();
    std::shared_ptr<UniformBuffer> createUniformBuffer(uint64_t size, bool inFlight = false);
    std::shared_ptr<Texture> createTexture(const std::string& filename, uint32_t mipLevels = 0);
    std::shared_ptr<Texture> createTexture(const uint8_t* data, uint32_t width, uint32_t height, uint32_t mipLevels = 0);
    std::shared_ptr<Sampler> createSampler(const SamplerInfos& infos = {});
    std::shared_ptr<PushConstants> createPushConstants(ShaderStages stages, const std::vector<PushConstantInfo>& infos);
    std::shared_ptr<Model> loadModel(const std::string& filename);
    std::shared_ptr<StorageImage> createStorageImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags additionalUsage = {});
    std::shared_ptr<RayTracingShaderProgram> createRayTracingShaderProgram();
    std::shared_ptr<RayTracingRenderPipeline> createRayTracingRenderPipeline(std::shared_ptr<RayTracingShaderProgram> shaderProgram);
    std::shared_ptr<AccelerationStructure> createAccelerationStructure();
    std::shared_ptr<RayTracingInstance> createRayTracingInstance(bool allow_update = true);

private:
    std::string path_;
    std::string shaderDir_;
    std::string textureDir_;
    std::string modelDir_;
};

} // namespace wen