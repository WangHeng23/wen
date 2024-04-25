#pragma once

#include "resources/image.hpp"
#include <vulkan/vulkan.hpp>

namespace wen {

struct Attachment {
    Attachment() = default;
    Attachment(const vk::AttachmentDescription& description, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect);    
    Attachment(Attachment&& rhs);
    void operator=(Attachment&& rhs);
    ~Attachment();

    std::unique_ptr<Image> image;
    vk::ImageView imageView;
};

class Renderer;

class Framebuffer {
    friend class Renderer;

public:
    Framebuffer(const Renderer& renderer, const std::vector<vk::ImageView>& imageViews);
    ~Framebuffer();

private:
    vk::Framebuffer framebuffer_;
};

class FramebufferStore {
    friend class Renderer;
    friend class DescriptorSet;

public:
    FramebufferStore(const Renderer& renderer);
    ~FramebufferStore();

private:
    std::vector<Attachment> attachments_;
    std::vector<std::unique_ptr<Framebuffer>> framebuffers_;
};

} // namespace wen