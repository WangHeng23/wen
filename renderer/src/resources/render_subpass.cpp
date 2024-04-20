#include "resources/render_subpass.hpp"
#include "resources/render_pass.hpp"

namespace wen {

RenderSubpass::RenderSubpass(const std::string& name, RenderPass& renderPass)
    : name(name), renderPass_(renderPass){};

vk::AttachmentReference RenderSubpass::createAttachmentReference(const std::string& name, vk::ImageLayout layout) {
    uint32_t attachment = renderPass_.getAttachmentIndex(name);
    vk::AttachmentReference reference = {};
    reference.setAttachment(attachment)
             .setLayout(layout);
    return reference;
}

void RenderSubpass::setOutputAttachment(const std::string& name, vk::ImageLayout layout) {
    outputAttachments_.push_back(createAttachmentReference(name, layout));
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
}

void RenderSubpass::setDepthStencilAttachment(const std::string& name, vk::ImageLayout layout) {
    depthStencilAttachment_ = createAttachmentReference(name, layout); 
}

vk::SubpassDescription RenderSubpass::build() {
    vk::SubpassDescription subpass = {};

    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
           .setColorAttachmentCount(outputAttachments_.size())
           .setColorAttachments(outputAttachments_);

    if (depthStencilAttachment_.has_value()) {
        subpass.setPDepthStencilAttachment(&depthStencilAttachment_.value());
    }

    return std::move(subpass);
}

} // namespace wen