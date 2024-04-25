#include "renderer.hpp"
#include "core/setting.hpp"
#include "core/logger.hpp"
#include "resources/render_pipeline.hpp"
#include "manager.hpp"

namespace wen {

Renderer::Renderer(std::shared_ptr<RenderPass> renderPass) {
    this->renderPass = renderPass;
    framebufferStore = std::make_unique<FramebufferStore>(*this);

    currentInFlight_ = 0;
    settings->currentInFlight = 0;
    commandBuffers_ = manager->commandPool->allocateCommandBuffers(settings->maxFramesInFlight);
    currentBuffer_ = commandBuffers_[0];

    imageAvailableSemaphores_.resize(settings->maxFramesInFlight);
    renderFinishedSemaphores_.resize(settings->maxFramesInFlight);
    inFlightFences_.resize(settings->maxFramesInFlight);

    vk::SemaphoreCreateInfo semaphore = {};
    vk::FenceCreateInfo fence = {};
    fence.setFlags(vk::FenceCreateFlagBits::eSignaled);
    auto& device = manager->device->device;
    for (uint32_t i = 0; i < settings->maxFramesInFlight; i++) {
        imageAvailableSemaphores_[i] = device.createSemaphore(semaphore);
        renderFinishedSemaphores_[i] = device.createSemaphore(semaphore);
        inFlightFences_[i] = device.createFence(fence);
        inFlightSubmitInfos_.emplace_back(); 
    }
}

Renderer::~Renderer() {
    waitIdle();
    auto& device = manager->device->device;
    for (uint32_t i = 0; i < settings->maxFramesInFlight; i++) {
        device.destroySemaphore(imageAvailableSemaphores_[i]);
        device.destroySemaphore(renderFinishedSemaphores_[i]);
        device.destroyFence(inFlightFences_[i]);
    }
    framebufferStore.reset();
    renderPass.reset();
}

void Renderer::setClearColor(const std::string& name, const vk::ClearValue& value) {
    uint32_t index = renderPass->getAttachmentIndex(name, false);
    renderPass->attachments[index].clearColor = value;
}

void Renderer::setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) {
    vk::Viewport viewport{x, y, width, height, minDepth, maxDepth};
    currentBuffer_.setViewport(0, {viewport});
}

void Renderer::setViewport(float x, float y, float width, float height) {
    setViewport(x, y, width, height, 0.0f, 1.0f);
}

void Renderer::setScissor(int x, int y, uint32_t width, uint32_t height) {
    vk::Rect2D scissor{{x, y}, {width, height}};
    currentBuffer_.setScissor(0, {scissor});
}

void Renderer::updateFramebuffers() {
    framebufferStore.reset();
    framebufferStore = std::make_unique<FramebufferStore>(*this);
}

void Renderer::updateSwapchain() {
    manager->recreateSwapchain();
    updateFramebuffers();
}

void Renderer::updateRenderPass() {
    renderPass->update();
    updateFramebuffers();
}

void Renderer::waitIdle() {
    manager->device->device.waitIdle();
}

void Renderer::acquireNextImage() {
    auto& device = manager->device->device;
    WEN_ASSERT(
        device.waitForFences(
            {inFlightFences_[currentInFlight_]}, true, std::numeric_limits<uint64_t>::max()
        ) == vk::Result::eSuccess,
        "Failed to wait for fence"
    )

    try {
        if (device.acquireNextImageKHR(
                manager->swapchain->swapchain,
                std::numeric_limits<uint64_t>::max(),
                imageAvailableSemaphores_[currentInFlight_],
                nullptr,
                &index_
            ) == vk::Result::eErrorOutOfDateKHR
        ) {
            updateSwapchain();
            return;
        }
    } catch (vk::OutOfDateKHRError) {
        updateSwapchain();
        return;
    }

    device.resetFences(inFlightFences_[currentInFlight_]);

    currentSubpass_ = 0;
    currentBuffer_.reset();
    vk::CommandBufferBeginInfo beginInfo = {};
    currentBuffer_.begin(beginInfo);
}

void Renderer::beginRenderPass() {
    std::vector<vk::ClearValue> clearValues;
    clearValues.reserve(renderPass->attachments.size());
    for (const auto& attachment : renderPass->attachments) {
        clearValues.push_back(attachment.clearColor);
    }

    vk::RenderPassBeginInfo renderPassBegin = {};
    auto [width, height] = settings->windowSize;
    vk::Rect2D renderArea(vk::Offset2D(0, 0), {width, height});
    renderPassBegin.setRenderPass(renderPass->renderPass)
        .setFramebuffer(framebufferStore->framebuffers_[index_]->framebuffer_)
        .setRenderArea(renderArea)
        .setClearValues(clearValues);

    currentBuffer_.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
}

void Renderer::beginRender() {
    acquireNextImage();
    beginRenderPass(); 
}

void Renderer::endRenderPass() {
    currentBuffer_.endRenderPass();
}

void Renderer::present() {
    currentBuffer_.end();

    std::vector<vk::Semaphore> waitSemaphores = {
        imageAvailableSemaphores_[currentInFlight_],
    };
    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
    };

    auto& submits = inFlightSubmitInfos_[currentInFlight_].emplace_back();
    submits.setWaitSemaphores(waitSemaphores)
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(currentBuffer_)
        .setSignalSemaphores(renderFinishedSemaphores_[currentInFlight_]);
    manager->device->graphicsQueue.submit(submits, inFlightFences_[currentInFlight_]);

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.setWaitSemaphores(renderFinishedSemaphores_[currentInFlight_])
        .setSwapchains(manager->swapchain->swapchain)
        .setImageIndices(index_)
        .setPResults(nullptr);

    try {
        auto res = manager->device->presentQueue.presentKHR(presentInfo);
        if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR) {
            updateSwapchain();
        }
    } catch (vk::OutOfDateKHRError) {
        updateSwapchain();
    }

    currentInFlight_ = (currentInFlight_ + 1) % settings->maxFramesInFlight;
    settings->currentInFlight = currentInFlight_;
    currentBuffer_ = commandBuffers_[currentInFlight_];
    inFlightSubmitInfos_[currentInFlight_].clear();
}

void Renderer::endRender() {
    endRenderPass();
    present();
}

void Renderer::bindPipeline(const std::shared_ptr<GraphicsRenderPipeline>& renderPipeline) {
    currentBuffer_.bindPipeline(bindPoint_, renderPipeline->pipeline);
}

void Renderer::bindDescriptorSets(const std::shared_ptr<GraphicsRenderPipeline>& renderPipeline) {
    if (!renderPipeline->descriptorSets.empty()) {
        std::vector<vk::DescriptorSet> descriptorSets;
        for (auto& descriptorSet : renderPipeline->descriptorSets) {
            descriptorSets.push_back(descriptorSet.value()->descriptorSets_[currentInFlight_]);
        }
        currentBuffer_.bindDescriptorSets(bindPoint_, renderPipeline->pipelineLayout, 0, descriptorSets, {});
    }
}

void Renderer::pushConstants(const std::shared_ptr<GraphicsRenderPipeline>& renderPipeline) {
    if (renderPipeline->pushConstants.has_value()) {
        auto pushConstants = renderPipeline->pushConstants.value();
        currentBuffer_.pushConstants(renderPipeline->pipelineLayout, pushConstants->range_.stageFlags, 0, pushConstants->size_, pushConstants->constants_.data());
    }
}

void Renderer::bindResources(std::shared_ptr<GraphicsRenderPipeline> renderPipeline) {
    bindPipeline(renderPipeline);
    bindDescriptorSets(renderPipeline);
    pushConstants(renderPipeline);
}

void Renderer::bindVertexBuffers(const std::vector<std::shared_ptr<VertexBuffer>>& vertexBuffers, uint32_t firstBinding) {
    std::vector<vk::Buffer> buffers;
    std::vector<vk::DeviceSize> offsets;
    for (const auto& vertexBuffer : vertexBuffers) {
        buffers.push_back(vertexBuffer->getBuffer()->buffer);
        offsets.push_back(0);
    }
    currentBuffer_.bindVertexBuffers(firstBinding, buffers, offsets);
}

void Renderer::bindVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, uint32_t binding) {
    bindVertexBuffers({vertexBuffer}, binding);
}

void Renderer::bindIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
    currentBuffer_.bindIndexBuffer(indexBuffer->getBuffer()->buffer, 0, indexBuffer->indexType);
}

void Renderer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    currentBuffer_.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void Renderer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    currentBuffer_.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void Renderer::drawModel(const std::shared_ptr<Model>& model, uint32_t instanceCount, uint32_t firstInstance) {
    currentBuffer_.drawIndexed(model->indexCount, instanceCount, model->offset().index, model->offset().vertex, firstInstance);
}

void Renderer::drawMesh(const std::shared_ptr<Mesh>& mesh, uint32_t instanceCount, uint32_t firstInstance) {
    currentBuffer_.drawIndexed(mesh->indices.size(), instanceCount, mesh->offset.index, mesh->offset.vertex, firstInstance);
}

void Renderer::nextSubpass() {
    currentBuffer_.nextSubpass(vk::SubpassContents::eInline);
    currentSubpass_++;
}

void Renderer::nextSubpass(const std::string& name) {
    auto index = renderPass->getSubpassIndex(name);
    while (currentSubpass_ != index) {
        nextSubpass();
    }
}

} // namespace wen