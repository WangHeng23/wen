#include "scenes/model_scene.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

Light::Light(std::shared_ptr<wen::Interface> interface) {
    uniformBuffer = interface->createUniformBuffer(sizeof(LightUniform));
    data = static_cast<LightUniform*>(uniformBuffer->getData());
    memset(data, 0, sizeof(LightUniform));
}

void ModelScene::initialize() {
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
            vk::AccessFlagBits::eNone,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
        }
    );
    renderPass->build();

    renderer = interface->createRenderer(std::move(renderPass));
    imguiLayer = interface->createImGuiLayer(renderer);

    // shader
    auto vertShader = interface->compileShader("model_scene/model.vert", wen::ShaderStage::eVertex);
    auto fragShader = interface->compileShader("model_scene/model.frag", wen::ShaderStage::eFragment);
    shaderProgram = interface->createGraphicsShaderProgram();
    shaderProgram->setVertexShader(vertShader);
    shaderProgram->setFragmentShader(fragShader);

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
        {
            .binding = 1,
            .inputRate = wen::InputRate::eInstance,
            .formats = {
                wen::VertexType::eFloat3, // offset
            }
        }
    });

    // descriptor set
    auto descriptorSet = interface->createDescriptorSet();
    descriptorSet->addDescriptors({
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex}, // camera
        {1, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex}, // light
    }).build();

    // ------------vertex input----------------
    // model
    model = interface->loadModel("dragon.obj");
    vertexBuffer = interface->createVertexBuffer(sizeof(wen::Vertex), model->vertexCount);
    indexBuffer = interface->createIndexBuffer(wen::IndexType::eUint32, model->indexCount);
    model->upload(vertexBuffer, indexBuffer);
    // offsets
    n = 3;
    int n2 = n / 2;
    for (int i = 0; i < n * n * n; i++) {
        offsets.push_back({i % n - n2, (i / n) % n - n2, ((i / n) / n) % n - n2});
    }
    offsetsBuffer = interface->createVertexBuffer(sizeof(glm::vec3), offsets.size());
    offsetsBuffer->upload(offsets);
    // ----------------------------------------

    // ------------descriptor set----------------
    // camera
    camera = std::make_unique<wen::Camera>();
    camera->data.position = {0.0f, 0.0f, -n - 3.0f};
    camera->direction = {0.0f, 0.0f, 1.0f};
    camera->upload();
    descriptorSet->bindUniform(0, camera->uniform);
    // light
    light = std::make_unique<Light>(interface);
    light->data->lights[0].position = glm::vec3(1.0f, 1.0f, 1.0f);
    light->data->lights[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
    light->data->lights[0].intensity = 1.0f;
    light->data->lightCount = 1;
    descriptorSet->bindUniform(1, light->uniformBuffer);
    // ----------------------------------------

    // render pipeline
    renderPipeline = interface->createGraphicsRenderPipeline(renderer, shaderProgram, "main subpass");
    renderPipeline->setVertexInput(vertexInput);
    renderPipeline->setDescriptorSet(descriptorSet);
    renderPipeline->compile({
        .topology = wen::Topology::eTriangleList,
        .polygonMode = wen::PolygonMode::eFill,
        .lineWidth = 2.0f,
        .cullMode = wen::CullMode::eNone,
        .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
        .depthTestEnable = true,
    });
}

void ModelScene::update(float ts) {
    camera->update(ImGui::GetIO().DeltaTime);

    static float time = 0;
    light->data->lights[0].position = glm::rotateY(glm::vec3(1.0f, 1.0f, 1.0f), time);
    time += 3 * ts;
}

void ModelScene::render() {
    renderer->setClearColor(wen::SWAPCHAIN_IMAGE_ATTACHMENT, {{clearColor.r, clearColor.g, clearColor.b, 1.0f}});

    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    renderer->getBindPoint(shaderProgram);
    renderer->bindResources(renderPipeline);
    renderer->setViewport(0, h, w, -h);
    renderer->setScissor(0, 0, w, h);
    renderer->bindVertexBuffers({vertexBuffer, offsetsBuffer});
    renderer->bindIndexBuffer(indexBuffer);
    renderer->drawModel(model, offsets.size(), 0);
}

void ModelScene::imgui() {
    ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
    ImGui::ColorEdit3("clear color", &clearColor.r);
    ImGui::ColorEdit3("light color", glm::value_ptr(light->data->lights[0].color));
    ImGui::SliderFloat("light intensity", &light->data->lights[0].intensity, 0.0f, 10.0f);
    ImGui::Text("model count: %d", n * n * n);
    ImGui::Text("vertex count: %d", model->vertexCount);
    ImGui::Text("index count: %d", model->indexCount);
    ImGui::Text("triangle count: %d", model->indexCount / 3);
}

void ModelScene::destroy() {
    camera.reset();
    light.reset();
    model.reset();
    shaderProgram.reset();
    renderPipeline.reset();
    vertexBuffer.reset();
    indexBuffer.reset();
    offsetsBuffer.reset();
}