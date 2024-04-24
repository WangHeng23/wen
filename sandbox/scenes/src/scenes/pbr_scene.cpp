#include "scenes/pbr_scene.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

PBRScene::Light::Light(wen::Interface& interface) {
    uniformBuffer = interface.createUniformBuffer(sizeof(LightUniform));
    data = static_cast<LightUniform*>(uniformBuffer->getData());
    memset(data, 0, sizeof(LightUniform));
}

PBRMaterial::PBRMaterial(wen::Interface& interface) {
    uniformBuffer = interface.createUniformBuffer(sizeof(MaterialUniform));
    data = static_cast<MaterialUniform*>(uniformBuffer->getData());
    memset(data, 0, sizeof(MaterialUniform));
}

void PBRScene::initialize() {
    auto renderPass = interface->createRenderPass();
    renderPass->addAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT, wen::AttachmentType::eColor);
    renderPass->addAttachment("depth buffer", wen::AttachmentType::eDepth);
    
    auto& subpass = renderPass->addSubpass("main subpass");
    subpass.setOutputAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT);
    subpass.setDepthStencilAttachment("depth buffer");
    
    renderPass->addSubpassDependency(
        wen::EXTERNAL_SUBPASS,
        "main subpass",
        {
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        },
        {
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eColorAttachmentRead
        }
    );
    renderPass->build();

    renderer = interface->createRenderer(std::move(renderPass));
    imguiLayer = interface->createImGuiLayer(renderer);

    // shader
    auto vertShader = interface->compileShader("pbr_scene/shader.vert", wen::ShaderStage::eVertex);
    auto fragShader = interface->compileShader("pbr_scene/shader.frag", wen::ShaderStage::eFragment);
    shaderProgram_ = interface->createGraphicsShaderProgram();
    shaderProgram_->setVertexShader(vertShader);
    shaderProgram_->setFragmentShader(fragShader);

    // vertex input
    auto vertexInput = interface->createVertexInput({
        {
            .binding = 0,
            .inputRate = wen::InputRate::eVertex,
            .formats = {
                wen::VertexType::eFloat3, // vertex
                wen::VertexType::eFloat3, // normal
                wen::VertexType::eFloat3, // color
            }
        },
    });

    // descriptor set
    auto descriptorSet = interface->createDescriptorSet();
    descriptorSet->addDescriptors({
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex|wen::ShaderStage::eFragment}, // camera
        {1, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment}, // material
        {2, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment}, // light
    }).build();

    // ------------vertex input----------------
    // model
    model_ = interface->loadModel("mori_knob.obj");
    vertexBuffer_ = interface->createVertexBuffer(sizeof(wen::Vertex), model_->vertexCount);
    indexBuffer_ = interface->createIndexBuffer(wen::IndexType::eUint32, model_->indexCount);
    model_->upload(vertexBuffer_, indexBuffer_);
    // ----------------------------------------

    // ------------descriptor set----------------
    // camera
    camera_ = std::make_unique<wen::Camera>(); 
    camera_->data.position = glm::vec3(0.0f, 0.0f, -3.0f);
    camera_->direction = glm::vec3(0.0f, 0.0f, 1.0f);
    camera_->upload();
    descriptorSet->bindUniform(0, camera_->uniformBuffer);
    // material
    material_ = std::make_unique<PBRMaterial>(*interface);
    material_->data->color = glm::vec3(0.5f);
    material_->data->roughness = 0.5f;
    material_->data->metallic = 0.7f;
    material_->data->reflectance = 0.7f;
    descriptorSet->bindUniform(1, material_->uniformBuffer);
    // light
    light_ = std::make_unique<Light>(*interface);
    light_->data->color = glm::vec3(1.0f, 1.0f, 1.0f);
    light_->data->direction = glm::normalize(glm::vec3(0, -1, 0));
    descriptorSet->bindUniform(2, light_->uniformBuffer);
    // ----------------------------------------

    // render pipeline
    renderPipeline_ = interface->createGraphicsRenderPipeline(renderer, shaderProgram_, "main subpass");
    renderPipeline_->setVertexInput(vertexInput);
    renderPipeline_->setDescriptorSet(descriptorSet);
    renderPipeline_->compile({
        .cullMode = wen::CullMode::eBack,
        .frontFace = wen::FrontFace::eCounterClockwise,
        .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
        .depthTestEnable = true,
    });
}

void PBRScene::update(float ts) {
    camera_->update(ts);

    static float time = 0;
    time += ts;
    float n = 1.0f;
    auto pos = glm::rotateY(glm::vec3(n, 0.0f, n), time);
    light_->data->pointLights[0].position = glm::vec3(0, pos.x, pos.z); // 绕X轴旋转
    light_->data->pointLights[1].position = glm::vec3(pos.x, 0, pos.z); // 绕Y轴旋转
    light_->data->pointLights[2].position = glm::vec3(pos.x, pos.z, 0); // 绕Z轴旋转
}

void PBRScene::render() {
    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    renderer->getBindPoint(shaderProgram_);
    renderer->bindResources(renderPipeline_);
    renderer->setViewport(0, h, w, -h);
    renderer->setScissor(0, 0, w, h);
    renderer->bindVertexBuffer(vertexBuffer_);
    renderer->bindIndexBuffer(indexBuffer_);
    renderer->drawModel(model_, 1, 0);
}

void PBRScene::imgui() {
    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
    ImGui::ColorEdit3("LightColor", &light_->data->color.r);
    ImGui::ColorEdit3("PointLight0", &light_->data->pointLights[0].color.r);
    ImGui::ColorEdit3("PointLight1", &light_->data->pointLights[1].color.r);
    ImGui::ColorEdit3("PointLight2", &light_->data->pointLights[2].color.r);
    ImGui::Separator();
    ImGui::ColorEdit3("Color", &material_->data->color.r);
    ImGui::SliderFloat("Roughness", &material_->data->roughness, 0, 1);
    ImGui::SliderFloat("Metallic", &material_->data->metallic, 0, 1);
    ImGui::SliderFloat("Reflectance", &material_->data->reflectance, 0, 1);
}

void PBRScene::destroy() {
    camera_.reset();
    light_.reset();
    model_.reset();
    material_.reset();
    shaderProgram_.reset();
    renderPipeline_.reset();
    vertexBuffer_.reset();
    indexBuffer_.reset();
}