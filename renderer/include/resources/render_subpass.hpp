#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class RenderPass;

class RenderSubpass final {
public:
    RenderSubpass(const std::string& name, RenderPass& renderPass);

    void setOutputAttachment(const std::string& name, vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription build();

private:
    vk::AttachmentReference createAttachmentReference(const std::string& name, vk::ImageLayout layout);

public:
    std::vector<vk::AttachmentReference> outputAttachments;
    std::vector<vk::PipelineColorBlendAttachmentState> outputColorBlendAttachments;

private:
    std::string name_;
    RenderPass& renderPass_;
};

} // namespace wen