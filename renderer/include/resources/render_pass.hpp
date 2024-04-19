#pragma once

#include "resources/render_subpass.hpp"
#include "utils/enums.hpp"
#include <vulkan/vulkan.hpp>

namespace wen {

struct AttachmentInfo {
    vk::AttachmentDescription writeAttachment = {};
    vk::ImageUsageFlags usage = {};
    vk::ImageAspectFlags aspect = {};
    vk::ClearValue clearColor = {};
};

class RenderPass final {
public:
    RenderPass();
    ~RenderPass();

    void addAttachment(const std::string& name, AttachmentType type);
    RenderSubpass& addSubpass(const std::string& name);
    void addSubpassDependency(const std::string& src, const std::string& dst, std::array<vk::PipelineStageFlags, 2> stage, std::array<vk::AccessFlags, 2> access);
    void build();
    void update();

    uint32_t getAttachmentIndex(const std::string& name) const;
    uint32_t getSubpassIndex(const std::string& name) const;

public:
    vk::RenderPass renderPass;
    std::vector<vk::AttachmentDescription> finalAttachments;
    std::vector<vk::SubpassDescription> finalSubpasses;
    std::vector<vk::SubpassDependency> finalDependencies;

    std::vector<AttachmentInfo> attachments;
    std::vector<std::unique_ptr<RenderSubpass>> subpasses;

private:
    std::map<std::string, uint32_t> attachmentIndices_;
    std::map<std::string, uint32_t> subpassIndices_;
};

} // namespace wen