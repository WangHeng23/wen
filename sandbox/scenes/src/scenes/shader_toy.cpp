#include "scenes/shader_toy.hpp"

ShaderToyInput::ShaderToyInput(wen::Interface& interface) {
    uniformBuffer = interface.createUniformBuffer(sizeof(ShaderToyInput));
    data = (ShadertoyInputUniform*)uniformBuffer->getData();
    memset(data, 0, sizeof(ShadertoyInputUniform));
}

void ShaderToy::initialize() {
    auto renderPass = interface->createRenderPass();
    renderPass->addAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT, wen::AttachmentType::eColor);
    
    auto& subpass = renderPass->addSubpass("main subpass");
    subpass.setOutputAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT);

    renderPass->build();

    renderer = interface->createRenderer(std::move(renderPass));
    imguiLayer = interface->createImGuiLayer(renderer);

    // shader
    auto vertShader = interface->compileShader("shader_toy/shader.vert", wen::ShaderStage::eVertex);
    auto fragShader = interface->compileShader("shader_toy/shader.frag", wen::ShaderStage::eFragment);
    shaderProgram_ = interface->createGraphicsShaderProgram();
    shaderProgram_->setVertexShader(vertShader);
    shaderProgram_->setFragmentShader(fragShader);

    // vertex input
    auto vertexInput = interface->createVertexInput({
        {
            .binding = 0, 
            .inputRate = wen::InputRate::eVertex,
            .formats = {
                wen::VertexType::eFloat2
            }
        },
    });

    // descriptor set
    auto descriptorSet = interface->createDescriptorSet();
    descriptorSet->addDescriptors({
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment}, // input
    });
    descriptorSet->build();

    // push constants
    pushConstants_ = interface->createPushConstants(
        wen::ShaderStage::eVertex | wen::ShaderStage::eFragment,
        {
            {"width", wen::ConstantType::eFloat},
            {"height", wen::ConstantType::eFloat},
        }
    );

    vertexBuffer_ = interface->createVertexBuffer(sizeof(glm::vec2), 4);
    vertexBuffer_->upload<glm::vec2>({
        { 1.0f,  1.0f},
        {-1.0f,  1.0f},
        { 1.0f, -1.0f},
        {-1.0f, -1.0f},
    });

    indexBuffer_ = interface->createIndexBuffer(wen::IndexType::eUint16, 6);
    indexBuffer_->upload<uint16_t>({
        0, 1, 2, 1, 2, 3,
    });

    input_ = std::make_unique<ShaderToyInput>(*interface);
    descriptorSet->bindUniform(0, input_->uniformBuffer);

    renderPipeline_ = interface->createGraphicsRenderPipeline(renderer, shaderProgram_, "main subpass");
    renderPipeline_->setVertexInput(vertexInput);
    renderPipeline_->setDescriptorSet(descriptorSet);
    renderPipeline_->setPushConstants(pushConstants_);
    renderPipeline_->compile({
        .cullMode = wen::CullMode::eNone,
        .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
        .depthTestEnable = false,
    });
}

void ShaderToy::update(float ts) {
    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    pushConstants_->pushConstant("width", &w);
    pushConstants_->pushConstant("height", &h);

    time_ += ts;
    input_->data->iTimeDelta = ts;
    input_->data->iTime = time_;
    input_->data->iFrameRate = ImGui::GetIO().Framerate;
    input_->data->iResolution = glm::vec3(w, h, 1.0f);
}

void ShaderToy::render() {
    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    renderer->getBindPoint(shaderProgram_);
    renderer->bindResources(renderPipeline_);
    renderer->bindVertexBuffer(vertexBuffer_);
    renderer->bindIndexBuffer(indexBuffer_);
    renderer->setViewport(0, h, w, -h);
    renderer->setScissor(0, 0, w, h);
    renderer->drawIndexed(6, 1, 0, 0, 0);
}

void ShaderToy::imgui() {
    ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
}

void ShaderToy::destroy() {}