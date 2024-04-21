#include "framebuffer.hpp"
#include "core/setting.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

Attachment::Attachment(const vk::AttachmentDescription& description, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
    image = std::make_unique<Image>(
        settings->windowSize.width, settings->windowSize.height,
        description.format,
        usage,
        description.samples,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    if (aspect == (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)) {
        aspect = vk::ImageAspectFlagBits::eDepth;
    }
    imageView = createImageView(image->image, description.format, aspect, 1);
}

Attachment::Attachment(Attachment&& rhs) {
    image = std::move(rhs.image);
    imageView = rhs.imageView;
    rhs.imageView = nullptr;
}

void Attachment::operator=(Attachment&& rhs) {
    image = std::move(rhs.image);
    imageView = rhs.imageView;
    rhs.imageView = nullptr;
}

Attachment::~Attachment() {
    if (image.get() != nullptr) {
        manager->device->device.destroyImageView(imageView);
        image.reset();
    }
}

Framebuffer::Framebuffer(const Renderer& renderer, const std::vector<vk::ImageView>& imageViews) {
    vk::FramebufferCreateInfo createInfo = {};
    createInfo.setRenderPass(renderer.renderPass->renderPass)
              .setWidth(settings->windowSize.width)
              .setHeight(settings->windowSize.height)
              .setLayers(1)
              .setAttachments(imageViews);
    framebuffer_ = manager->device->device.createFramebuffer(createInfo);
}

Framebuffer::~Framebuffer() {
    manager->device->device.destroyFramebuffer(framebuffer_);
}

FramebufferStore::FramebufferStore(const Renderer& renderer) {
    uint32_t count = renderer.renderPass->finalAttachments.size();
    std::vector<vk::ImageView> imageViews(count);
    attachments_.resize(count);
    int index = 0;

    auto attachment = renderer.renderPass->attachments[0];
    if (attachment.readAttachment.has_value()) {
        attachments_[0] = std::move(
            Attachment(
                attachment.writeAttachment,
                attachment.usage,
                attachment.aspect
            )
        );
        index = renderer.renderPass->attachments.size();
    } else {
        index = 0;
    }

    for (auto& [name, idx] : renderer.renderPass->getAttachmentIndices()) {
        auto& attachment = renderer.renderPass->attachments[idx];
        if (idx != 0) {
            attachments_[idx] = std::move(
                Attachment(
                    attachment.writeAttachment,
                    attachment.usage,
                    attachment.aspect
                )
            );
            if (attachment.readAttachment.has_value()) {
                attachments_[renderer.renderPass->getAttachmentIndex(name, true)] = std::move(
                    Attachment(
                        attachment.readAttachment.value(),
                        attachment.read_usage,
                        attachment.aspect
                    )
                );
            }
        }
    }

    for (uint32_t i = 0; i < count; i++) {
        imageViews[i] = attachments_[i].imageView;
    }
    for (auto imageView : manager->swapchain->imageViews) {
        imageViews[index] = imageView;
        framebuffers_.push_back(std::make_unique<Framebuffer>(renderer, imageViews));
    }
}

FramebufferStore::~FramebufferStore() {
    attachments_.clear();
    framebuffers_.clear();
}

} // namespace wen