#include "scenes/ray_marching.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

RayMarchingInfo::RayMarchingInfo(wen::Interface& interface) {
    uniformBuffer = interface.createUniformBuffer(sizeof(RayMarchingUniform));
    data = static_cast<RayMarchingUniform*>(uniformBuffer->getData());
    memset(data, 0, sizeof(RayMarchingUniform));
}

void RayMarching::initialize() {
    auto renderPass = interface->createRenderPass();
    renderPass->addAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT, wen::AttachmentType::eColor);

    auto& subpass = renderPass->addSubpass("main subpass");
    subpass.setOutputAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT);

    renderPass->build();

    renderer = interface->createRenderer(std::move(renderPass));
    imguiLayer = interface->createImGuiLayer(renderer);

    auto vertShader = interface->compileShader("ray_marching/shader.vert", wen::ShaderStage::eVertex);
    auto fragShader = interface->compileShader("ray_marching/shader.frag", wen::ShaderStage::eFragment);
    shaderProgram_ = interface->createGraphicsShaderProgram();
    shaderProgram_->setVertexShader(vertShader);
    shaderProgram_->setFragmentShader(fragShader);

    auto descriptorSet = interface->createDescriptorSet();
    descriptorSet->addDescriptors({
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment}, // camera
        {1, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment}, // ray marching info
    }).build();

    camera_ = std::make_unique<wen::Camera>(); 
    camera_->data.position = glm::vec3(0.0f, 1.0f, -6.0f);
    camera_->direction = glm::vec3(0.0f, 0.0f, 1.0f);
    camera_->upload();
    descriptorSet->bindUniform(0, camera_->uniformBuffer);

    info_ = std::make_unique<RayMarchingInfo>(*interface);
    info_->data->maxSteps = 100;
    info_->data->maxDist = 100.0f;
    info_->data->epsillonDist = 0.0001f;
    info_->data->sphere = glm::vec4(0.0f, 1.0f, 2.0f, 1.0f);
    info_->data->intensity = 1.0f;
    descriptorSet->bindUniform(1, info_->uniformBuffer);

    renderPipeline_ = interface->createGraphicsRenderPipeline(renderer, shaderProgram_, "main subpass");
    renderPipeline_->setDescriptorSet(descriptorSet);
    renderPipeline_->compile({
        .cullMode = wen::CullMode::eNone,
    });
}

void RayMarching::update(float ts) {
    camera_->update(ts);

    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    info_->data->windowSize = glm::vec2(w, h);
    static float time = 0.0f;
    time += 2 * ts;
    info_->data->light = glm::vec3(info_->data->sphere.x, 2.0f, info_->data->sphere.z) + glm::rotateY(glm::vec3(1.2f, 1.2f, 1.2f), time);;
}

void RayMarching::render() {
    renderer->getBindPoint(shaderProgram_);
    renderer->bindResources(renderPipeline_);
    renderer->draw(3, 2, 0, 0);
}

void RayMarching::imgui() {
    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
    ImGui::SliderInt("max steps", &info_->data->maxSteps, 1, 1000);
    ImGui::SliderFloat("max dist", &info_->data->maxDist, 0.001, 1000);
    ImGui::SliderFloat("epsillon dist", &info_->data->epsillonDist, 0.0001, 0.01);
    ImGui::SliderFloat3("sphere pos", &info_->data->sphere.x, -3, 3);
    ImGui::SliderFloat("sphere size", &info_->data->sphere.w, 0.001, 2);
    ImGui::SliderFloat("intensity", &info_->data->intensity, 0.5, 5);
}

void RayMarching::destroy() {
    info_.reset();
    camera_.reset();
    shaderProgram_.reset();
    renderPipeline_.reset();
}