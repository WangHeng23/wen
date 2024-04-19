#pragma once

#include "resources/render_pass.hpp"
#include "resources/shader_program.hpp"
#include "framebuffer.hpp"
#include "storage/vertex_buffer.hpp"
#include "storage/index_buffer.hpp"

namespace wen {

class Renderer final {
public:
    Renderer(std::shared_ptr<RenderPass> renderPass);
    ~Renderer();

    void acquireNextImage();
    void beginRenderPass();
    void beginRender();
    void endRenderPass(); 
    void present();
    void endRender();

public:
    void setClearColor(const std::string& name, const vk::ClearValue& value);
    void setViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
    void setViewport(float x, float y, float width, float height);
    void setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

public:
    template <class ShaderProgramClass>
    void getBindPoint(std::shared_ptr<ShaderProgramClass> shaderProgram) {
        bindPoint_ = ShaderProgramBindPoint<ShaderProgramClass>::bindPoint;
    }
    void bindPipeline(const std::shared_ptr<GraphicsRenderPipeline>& renderPipeline);
    void bindDescriptorSets(const std::shared_ptr<GraphicsRenderPipeline>& renderPipeline);
    void bindResources(std::shared_ptr<GraphicsRenderPipeline> renderPipeline);
    void bindVertexBuffers(const std::vector<std::shared_ptr<VertexBuffer>>& vertexBuffers, uint32_t firstBinding = 0);
    void bindVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, uint32_t binding = 0);
    void bindIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);
    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

    void nextSubpass();
    void toNextSubpass(const std::string& name);

public:
    vk::CommandBuffer getCurrentBuffer() { return currentBuffer_; }
    void updateFramebuffers();
    void updateSwapchain();
    void updateRenderPass();
    void waitIdle();

public:
    std::shared_ptr<RenderPass> renderPass;
    std::unique_ptr<FramebufferStore> framebufferStore;

private:
    vk::PipelineBindPoint bindPoint_;

    vk::CommandBuffer currentBuffer_;
    std::vector<vk::CommandBuffer> commandBuffers_;

    std::vector<vk::Semaphore> imageAvailableSemaphores_;
    std::vector<vk::Semaphore> renderFinishedSemaphores_;
    std::vector<vk::Fence> inFlightFences_;
    std::vector<std::vector<vk::SubmitInfo>> inFlightSubmitInfos_;

    uint32_t index_;
    uint32_t currentInFlight_;
    uint32_t currentSubpass_;
};

} // namespace wen