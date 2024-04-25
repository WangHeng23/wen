#include "scenes/model_scene.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

ModelScene::Light::Light(std::shared_ptr<wen::Interface> interface) {
    uniformBuffer = interface->createUniformBuffer(sizeof(LightUniform));
    data = static_cast<LightUniform*>(uniformBuffer->getData());
    memset(data, 0, sizeof(LightUniform));
}

void ModelScene::initialize() {
    auto renderPass = interface->createRenderPass();
    renderPass->addAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT, wen::AttachmentType::eColor);
    renderPass->addAttachment("depth buffer", wen::AttachmentType::eDepth);
    renderPass->addAttachment("position buffer", wen::AttachmentType::eRGBA32F);
    renderPass->addAttachment("normal buffer", wen::AttachmentType::eRGBA32F);
    renderPass->addAttachment("color buffer", wen::AttachmentType::eRGBA32F);
    
    auto& mainSubpass = renderPass->addSubpass("main subpass");
    mainSubpass.setOutputAttachment("position buffer");
    mainSubpass.setOutputAttachment("normal buffer");
    mainSubpass.setOutputAttachment("color buffer");
    mainSubpass.setDepthStencilAttachment("depth buffer");

    auto& postSubpass = renderPass->addSubpass("post subpass");
    postSubpass.setInputAttachment("position buffer");
    postSubpass.setInputAttachment("normal buffer");
    postSubpass.setInputAttachment("color buffer");
    postSubpass.setOutputAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT);
    
    renderPass->addSubpassDependency(
        "main subpass",
        "post subpass",
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
    auto mainVertShader = interface->compileShader("model_scene/main.vert", wen::ShaderStage::eVertex);
    auto mainFragShader = interface->compileShader("model_scene/main.frag", wen::ShaderStage::eFragment);
    mainShaderProgram = interface->createGraphicsShaderProgram();
    mainShaderProgram->setVertexShader(mainVertShader);
    mainShaderProgram->setFragmentShader(mainFragShader);
    auto postVertShader = interface->compileShader("model_scene/post.vert", wen::ShaderStage::eVertex);
    auto postFragShader = interface->compileShader("model_scene/post.frag", wen::ShaderStage::eFragment);
    postShaderProgram = interface->createGraphicsShaderProgram();
    postShaderProgram->setVertexShader(postVertShader);
    postShaderProgram->setFragmentShader(postFragShader);

    // vertex input
    auto vertexInput = interface->createVertexInput({
        {
            .binding = 0,
            .inputRate = wen::InputRate::eVertex,
            .formats = {
                wen::VertexType::eFloat3, // position
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
    auto mainDescriptorSet = interface->createDescriptorSet();
    mainDescriptorSet->addDescriptors({
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex}, // camera
    }).build();
    auto postDescriptorSet = interface->createDescriptorSet();
    postDescriptorSet->addDescriptors({
        {0, wen::DescriptorType::eInputAttachment, 3, wen::ShaderStage::eFragment}, // input attachment
        {1, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment}, // camera
        {2, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment}, // light
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
    // main descriptor set
    // camera
    camera = std::make_unique<wen::Camera>();
    camera->data.position = {0.0f, 0.0f, -n - 3.0f};
    camera->direction = {0.0f, 0.0f, 1.0f};
    camera->upload();
    mainDescriptorSet->bindUniform(0, camera->uniformBuffer);
    // post descriptor set
    // sampler
    auto sampler = interface->createSampler();
    postDescriptorSet->bindInputAttachments(
        0, 
        renderer,
        {
            {"position buffer", sampler},
            {"normal buffer", sampler},
            {"color buffer", sampler},
        }
    );
    // camera
    postDescriptorSet->bindUniform(1, camera->uniformBuffer);
    // light
    light = std::make_unique<Light>(interface);
    light->data->lights[0].position = glm::vec3(1.0f, 1.0f, 1.0f);
    light->data->lights[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
    light->data->lights[0].intensity = 1.0f;
    light->data->lightCount = 1;
    postDescriptorSet->bindUniform(2, light->uniformBuffer);
    // ----------------------------------------

    // render pipeline
    mainRenderPipeline = interface->createGraphicsRenderPipeline(renderer, mainShaderProgram, "main subpass");
    mainRenderPipeline->setVertexInput(vertexInput);
    mainRenderPipeline->setDescriptorSet(mainDescriptorSet);
    mainRenderPipeline->compile({
        .cullMode = wen::CullMode::eBack,
        .frontFace = wen::FrontFace::eCounterClockwise,
        .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
        .depthTestEnable = true,
    });
    postRenderPipeline = interface->createGraphicsRenderPipeline(renderer, postShaderProgram, "post subpass");
    postRenderPipeline->setDescriptorSet(postDescriptorSet);
    postRenderPipeline->compile({
        .cullMode = wen::CullMode::eNone,
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
    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    renderer->getBindPoint(mainShaderProgram);
    renderer->bindResources(mainRenderPipeline);
    renderer->setViewport(0, h, w, -h);
    renderer->setScissor(0, 0, w, h);
    renderer->bindVertexBuffers({vertexBuffer, offsetsBuffer});
    renderer->bindIndexBuffer(indexBuffer);
    renderer->drawModel(model, offsets.size(), 0);
    renderer->nextSubpass();
    renderer->getBindPoint(postShaderProgram);
    renderer->bindResources(postRenderPipeline);
    renderer->drawIndexed(3, 2, 0, 0, 0);
}

void ModelScene::imgui() {
    ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
    ImGui::ColorEdit3("light color", glm::value_ptr(light->data->lights[0].color));
    ImGui::SliderFloat("light intensity", &light->data->lights[0].intensity, 0.0f, 10.0f);
    ImGui::Separator();
    ImGui::Text("model count: %d", n * n * n);
    ImGui::Text("vertex count: %d", model->vertexCount);
    ImGui::Text("index count: %d", model->indexCount);
    ImGui::Text("triangle count: %d", model->indexCount / 3);
}

void ModelScene::destroy() {
    camera.reset();
    light.reset();
    model.reset();
    mainShaderProgram.reset();
    postShaderProgram.reset();
    mainRenderPipeline.reset();
    postRenderPipeline.reset();
    vertexBuffer.reset();
    indexBuffer.reset();
    offsetsBuffer.reset();
}