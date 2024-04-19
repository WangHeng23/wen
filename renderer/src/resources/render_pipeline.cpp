#include "resources/render_pipeline.hpp"
#include "utils/utils.hpp"
#include "core/setting.hpp"
#include "renderer.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

namespace wen {

void RenderPipeline::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo info = {};

    std::vector<vk::DescriptorSetLayout> layouts;
    layouts.reserve(descriptorSets.size());
    for (auto& set : descriptorSets) {
        if (!set.has_value()) {
            WEN_ERROR("Miss descriptor set at creating pipeline layout");
        } 
        layouts.push_back(set.value()->descriptorLayout_);
    }

    info.setSetLayouts(layouts)
        .setPushConstantRanges(nullptr);

    pipelineLayout = manager->device->device.createPipelineLayout(info);
}

RenderPipeline::~RenderPipeline() {
    manager->device->device.destroyPipeline(pipeline);
    manager->device->device.destroyPipelineLayout(pipelineLayout);
}

GraphicsRenderPipeline::GraphicsRenderPipeline(std::weak_ptr<Renderer> renderer, std::shared_ptr<GraphicsShaderProgram> shaderProgram, const std::string& subpassName) {
    renderer_ = renderer;
    shaderProgram_ = shaderProgram;
    subpassName_ = subpassName; 
}

GraphicsRenderPipeline::~GraphicsRenderPipeline() {
    shaderProgram_.reset();
}

static vk::PipelineShaderStageCreateInfo createShaderStage(
    vk::ShaderStageFlagBits stage,
    vk::ShaderModule module,
    const std::string& entry
) {
    vk::PipelineShaderStageCreateInfo info = {};
    info.setPSpecializationInfo(nullptr)
        .setStage(stage)
        .setModule(module)
        .setPName(entry.c_str());

    return info;
}

void GraphicsRenderPipeline::setVertexInput(std::shared_ptr<VertexInput> vertexInput) {
    vertexInput_.reset();
    vertexInput_ = std::move(vertexInput);
}

void GraphicsRenderPipeline::compile(const GraphicsRenderPipelineOptions& options) {
    // 0. shader
    std::vector<vk::PipelineShaderStageCreateInfo> stages = {
        createShaderStage(
            vk::ShaderStageFlagBits::eVertex,
            shaderProgram_->vertexShader_.shader->module.value(),
            shaderProgram_->vertexShader_.entry
        ),
        createShaderStage(
            vk::ShaderStageFlagBits::eFragment,
            shaderProgram_->fragmentShader_.shader->module.value(),
            shaderProgram_->fragmentShader_.entry
        ),
    };

    // 1. vertex input
    vk::PipelineVertexInputStateCreateInfo vertexInput = {};
    if (vertexInput_.get() != nullptr) {
        vertexInput.setVertexBindingDescriptions(vertexInput_->bindingDescriptions_)
                   .setVertexAttributeDescriptions(vertexInput_->attributeDescriptions_);
    }

    // 2. input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.setPrimitiveRestartEnable(false)
                 .setTopology(convert<vk::PrimitiveTopology>(options.topology));
    
    // 3. viewport
    vk::PipelineViewportStateCreateInfo viewport = {};
    auto [width, height] = settings->windowSize;
    float w = static_cast<float>(width), h = static_cast<float>(height);
    vk::Viewport view(0.0f, 0.0f, w, h, 0.0f, 1.0f);
    vk::Rect2D scissor({0, 0}, {width, height});
    viewport.setViewportCount(1)
            .setViewports(view)
            .setScissorCount(1)
            .setScissors(scissor);
    
    // 4. rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.setDepthClampEnable(true)
              .setRasterizerDiscardEnable(false)
              .setPolygonMode(convert<vk::PolygonMode>(options.polygonMode))
              .setLineWidth(options.lineWidth)
              .setCullMode(convert<vk::CullModeFlags>(options.cullMode))
              .setFrontFace(convert<vk::FrontFace>(options.frontFace))
              .setDepthBiasEnable(false)
              .setDepthBiasConstantFactor(0.0f)
              .setDepthBiasClamp(false)
              .setDepthBiasSlopeFactor(0.0f);

    // 5. multisample
    vk::PipelineMultisampleStateCreateInfo multisample = {};
    multisample.setSampleShadingEnable(true)
               .setRasterizationSamples(vk::SampleCountFlagBits::e1)
               .setMinSampleShading(0.2f)
               .setPSampleMask(nullptr)
               .setAlphaToCoverageEnable(false)
               .setAlphaToOneEnable(false);

    // 6. depth and stencil
    vk::PipelineDepthStencilStateCreateInfo depthStencil = {};

    // 7. color blending
    vk::PipelineColorBlendStateCreateInfo colorBlend = {};
    auto locked = renderer_.lock();
    uint32_t subpassIndex = locked->renderPass->getSubpassIndex(subpassName_);
    auto subpass = *locked->renderPass->subpasses[subpassIndex];
    colorBlend.setLogicOpEnable(false)
              .setAttachments(subpass.outputColorBlendAttachments);

    // 8. dynamic state
    vk::PipelineDynamicStateCreateInfo dynamic = {};
    dynamic.setDynamicStateCount(options.dynamicStates.size())
           .setDynamicStates(options.dynamicStates);

    createPipelineLayout();

    // 10. pipeline
    vk::GraphicsPipelineCreateInfo info;
    info.setStageCount(stages.size())
        .setPStages(stages.data())
        .setPVertexInputState(&vertexInput)
        .setPInputAssemblyState(&inputAssembly)
        .setPTessellationState(nullptr)
        .setPViewportState(&viewport)
        .setPRasterizationState(&rasterizer)
        .setPMultisampleState(&multisample)
        .setPDepthStencilState(&depthStencil)
        .setPColorBlendState(&colorBlend)
        .setPDynamicState(&dynamic)
        .setLayout(pipelineLayout)
        .setRenderPass(locked->renderPass->renderPass)
        .setSubpass(subpassIndex)
        .setBasePipelineHandle(nullptr)
        .setBasePipelineIndex(0);
    
    locked.reset();
    
    pipeline = manager->device->device.createGraphicsPipeline(nullptr, info).value;
} 

} // namespace wen