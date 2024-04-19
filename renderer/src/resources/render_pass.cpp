#include "resources/render_pass.hpp"
#include "core/setting.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

namespace wen {

RenderPass::RenderPass() {
    addAttachment(SWAPCHAIN_IMAGE_ATTACHMENT, AttachmentType::eColor);
    auto& attachment = attachments[0];
    attachment.writeAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
}

void RenderPass::addAttachment(const std::string& name, AttachmentType type) {
    attachmentIndices_.insert(std::make_pair(name, attachments.size())); 
    auto& attachment = attachments.emplace_back();
    attachment.writeAttachment
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    
    switch (type) {
        case AttachmentType::eColor:
            attachment.writeAttachment.format = manager->swapchain->format.format;
            attachment.writeAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
            attachment.usage = vk::ImageUsageFlagBits::eColorAttachment;
            attachment.aspect = vk::ImageAspectFlagBits::eColor;
            attachment.clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
            break;
        case AttachmentType::eDepth:
            break;
    }
}

RenderSubpass& RenderPass::addSubpass(const std::string& name) {
    subpassIndices_.insert(std::make_pair(name, subpasses.size()));
    subpasses.push_back(std::make_unique<RenderSubpass>(name, *this));
    return *subpasses.back();
}

void RenderPass::addSubpassDependency(const std::string& src, const std::string& dst, std::array<vk::PipelineStageFlags, 2> stage, std::array<vk::AccessFlags, 2> access) {
    finalDependencies.push_back({
        src == EXTERNAL_SUBPASS ? VK_SUBPASS_EXTERNAL : getSubpassIndex(src),
        getSubpassIndex(dst),
        stage[0],
        stage[1],
        access[0],
        access[1]
    });
}

void RenderPass::build() {
    vk::RenderPassCreateInfo info = {};

    finalAttachments.clear();
    finalAttachments.reserve(attachments.size());
    for (const auto& attachment : attachments) {
        finalAttachments.push_back(attachment.writeAttachment);
    }

    finalSubpasses.clear();
    finalSubpasses.reserve(subpasses.size());
    for (const auto& subpass : subpasses) {
        finalSubpasses.push_back(subpass->build());
    }

    info.setAttachmentCount(finalAttachments.size())
        .setAttachments(finalAttachments)
        .setSubpassCount(finalSubpasses.size())
        .setSubpasses(finalSubpasses)
        .setDependencyCount(finalDependencies.size())
        .setDependencies(finalDependencies);

    renderPass = manager->device->device.createRenderPass(info);
}

void RenderPass::update() {
    manager->device->device.destroyRenderPass(renderPass);
    build();
}

uint32_t RenderPass::getAttachmentIndex(const std::string& name) const {
    auto index = attachmentIndices_.find(name);
    if (index == attachmentIndices_.end()) {
        WEN_ERROR("Attachment \"{}\" not found", name)
        return -1u;
    }
    return index->second;
}

uint32_t RenderPass::getSubpassIndex(const std::string& name) const {
    auto index = subpassIndices_.find(name);
    if (index == subpassIndices_.end()) {
        WEN_ERROR("Subpass \"{}\" not found", name)
        return -1u;
    }
    return index->second;
}

RenderPass::~RenderPass() {
    manager->device->device.destroyRenderPass(renderPass);
}

} // namespace wen