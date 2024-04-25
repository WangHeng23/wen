#include "resources/render_pass.hpp"
#include "core/setting.hpp"
#include "core/logger.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

void RenderPass::addAttachment(const std::string& name, AttachmentType type) {
    attachmentIndices_.insert(std::make_pair(name, attachments.size())); 
    auto& attachment = attachments.emplace_back();
    attachment.writeAttachment
        .setSamples(convert<vk::SampleCountFlagBits>(settings->msaaSamples))
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
            if (settings->msaa()) {
                attachment.readAttachment = attachment.writeAttachment;
                attachment.readAttachment->samples = vk::SampleCountFlagBits::e1;
                attachment.readAttachment->loadOp = vk::AttachmentLoadOp::eDontCare;
                attachment.read_usage = vk::ImageUsageFlagBits::eColorAttachment;
                attachment.read_offset = readAttachments.size();
                readAttachments.push_back(attachment);
            }
            break;
        case AttachmentType::eDepth:
            attachment.writeAttachment.format = findDepthFormat();
            attachment.writeAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            attachment.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
            attachment.aspect = vk::ImageAspectFlagBits::eDepth;
            attachment.clearColor = {{1.0f, 0}};
            if (attachment.writeAttachment.format == vk::Format::eD32SfloatS8Uint ||
                attachment.writeAttachment.format == vk::Format::eD24UnormS8Uint) {
                attachment.aspect |= vk::ImageAspectFlagBits::eStencil;
            }
            attachment.writeAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
            break;
        case AttachmentType::eRGBA32F:
            attachment.writeAttachment.format = vk::Format::eR32G32B32A32Sfloat;
            attachment.writeAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            attachment.usage = vk::ImageUsageFlagBits::eColorAttachment;
            attachment.aspect = vk::ImageAspectFlagBits::eColor;
            attachment.clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
            if (settings->msaa()) {
                attachment.readAttachment = attachment.writeAttachment;
                attachment.readAttachment->samples = vk::SampleCountFlagBits::e1;
                attachment.readAttachment->loadOp = vk::AttachmentLoadOp::eDontCare;
                attachment.writeAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
                attachment.read_usage = vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled;
                attachment.read_offset = readAttachments.size();
                readAttachments.push_back(attachment);
            } else {
                attachment.usage |= vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled;
            }
            break;
    }

    static bool first = true;
    if (first) {
        auto& attachment = attachments[0];
        if (attachment.readAttachment.has_value()) {
            readAttachments[0].readAttachment->finalLayout = vk::ImageLayout::ePresentSrcKHR;
        } else {
            attachment.writeAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
        }
        first = false;
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
    finalAttachments.reserve(attachments.size() + readAttachments.size());
    for (const auto& attachment : attachments) {
        finalAttachments.push_back(attachment.writeAttachment);
    }
    if (settings->msaa()) {
        for (const auto& attachment : readAttachments) {
            finalAttachments.push_back(attachment.readAttachment.value());
        }
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

uint32_t RenderPass::getAttachmentIndex(const std::string& name, bool read) const {
    auto index = attachmentIndices_.find(name);
    if (index == attachmentIndices_.end()) {
        WEN_ERROR("Attachment \"{}\" not found", name)
        return -1u;
    }

    if (read && attachments[index->second].readAttachment.has_value()) {
        return attachments[index->second].read_offset + attachments.size();
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