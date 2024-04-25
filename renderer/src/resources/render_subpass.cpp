#include "resources/render_subpass.hpp"
#include "resources/render_pass.hpp"
#include "core/setting.hpp"

namespace wen {

RenderSubpass::RenderSubpass(const std::string& name, RenderPass& renderPass)
    : name(name), renderPass_(renderPass){};

vk::AttachmentReference RenderSubpass::createAttachmentReference(const std::string& name, vk::ImageLayout layout, bool read) {
    uint32_t attachment = renderPass_.getAttachmentIndex(name, read);
    vk::AttachmentReference reference = {};
    reference.setAttachment(attachment)
             .setLayout(layout);
    return reference;
}

void RenderSubpass::setOutputAttachment(const std::string& name, vk::ImageLayout layout) {
    outputAttachments_.push_back(createAttachmentReference(name, layout, false));
    outputColorBlendAttachments.push_back({
        false,
        vk::BlendFactor::eZero,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eZero,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    });
    resolveAttachments_.push_back(createAttachmentReference(name, layout, true));
}

void RenderSubpass::setDepthStencilAttachment(const std::string& name, vk::ImageLayout layout) {
    depthStencilAttachment_ = createAttachmentReference(name, layout, false); 
}

void RenderSubpass::setInputAttachment(const std::string& name, vk::ImageLayout layout) {
    inputAttachments_.push_back(createAttachmentReference(name, layout, true));
}

vk::SubpassDescription RenderSubpass::build() {
    vk::SubpassDescription subpass = {};

    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
           .setColorAttachmentCount(outputAttachments_.size())
           .setColorAttachments(outputAttachments_)
           .setInputAttachmentCount(inputAttachments_.size())
           .setInputAttachments(inputAttachments_);

    if (depthStencilAttachment_.has_value()) {
        subpass.setPDepthStencilAttachment(&depthStencilAttachment_.value());
    }

    if (settings->msaa()) {
        subpass.setResolveAttachments(resolveAttachments_);
    }

    return std::move(subpass);
}

} // namespace wen