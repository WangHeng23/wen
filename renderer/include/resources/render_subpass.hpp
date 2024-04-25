#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class RenderPass;

class RenderSubpass final {
public:
    RenderSubpass(const std::string& name, RenderPass& renderPass);

    void setOutputAttachment(const std::string& name, vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal);
    void setDepthStencilAttachment(const std::string& name, vk::ImageLayout layout = vk::ImageLayout::eDepthStencilAttachmentOptimal);
    void setInputAttachment(const std::string& name, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::SubpassDescription build();

private:
    vk::AttachmentReference createAttachmentReference(const std::string& name, vk::ImageLayout layout, bool read);

public:
    std::string name;
    std::vector<vk::PipelineColorBlendAttachmentState> outputColorBlendAttachments;

private:
    RenderPass& renderPass_;
    std::vector<vk::AttachmentReference> outputAttachments_;
    std::optional<vk::AttachmentReference> depthStencilAttachment_;
    std::vector<vk::AttachmentReference> resolveAttachments_;
    std::vector<vk::AttachmentReference> inputAttachments_;
};

} // namespace wen