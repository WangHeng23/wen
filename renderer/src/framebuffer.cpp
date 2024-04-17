#include "framebuffer.hpp"
#include "core/setting.hpp"
#include "manager.hpp"

namespace wen {

Framebuffer::Framebuffer(const Renderer& renderer, uint32_t index) {
    std::vector<vk::ImageView> imageViews = {
        manager->swapchain->imageViews[index],
    };

    vk::FramebufferCreateInfo info = {};
    info.setRenderPass(renderer.renderPass->renderPass)
        .setWidth(settings->windowSize.width)
        .setHeight(settings->windowSize.height)
        .setLayers(1)
        .setAttachments(imageViews);

    framebuffer_ = manager->device->device.createFramebuffer(info);
}

Framebuffer::~Framebuffer() {
    manager->device->device.destroyFramebuffer(framebuffer_);
}

FramebufferStore::FramebufferStore(const Renderer& renderer) {
    for (uint32_t i = 0; i < manager->swapchain->imageCount; i++) {
        framebuffers_.push_back(std::make_unique<Framebuffer>(renderer, i));
    }
}

FramebufferStore::~FramebufferStore() {
    framebuffers_.clear();
}

} // namespace wen