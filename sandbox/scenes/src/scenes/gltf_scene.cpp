#include "scenes/gltf_scene.hpp"

void GLTFScene::initialize() {
    auto renderPass = interface->createRenderPass();
    renderPass->addAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT, wen::AttachmentType::eColor);
    renderPass->addAttachment("depth buffer", wen::AttachmentType::eDepth);

    auto& subpass = renderPass->addSubpass("main subpass");
    subpass.setDepthStencilAttachment("depth buffer");
    subpass.setOutputAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT);

    renderPass->build();

    renderer = interface->createRenderer(std::move(renderPass));
    imguiLayer = interface->createImGuiLayer(renderer);

    scene_ = interface->loadGLTFScene("Sponza/glTF/Sponza.gltf");
    auto as = interface->createAccelerationStructure();
    as->addScene(scene_);
    as->build(false, false);
    as.reset();

    descriptorSet_ = interface->createDescriptorSet();
    descriptorSet_->addDescriptors({
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex}, // camera
    }).build();

    camera_ = std::make_unique<wen::Camera>();
    camera_->data.position = {0.0f, 1.0f, -3.5f};
    camera_->direction = {0.0f, 0.0f, 1.0f};
    camera_->upload();
    descriptorSet_->bindUniform(0, camera_->uniformBuffer);

    vertexInput_ = interface->createVertexInput({
        {
            .binding = 0,
            .inputRate = wen::InputRate::eVertex,
            .formats = {
                wen::VertexType::eFloat3,
            }
         }
    });

    auto vertShader = interface->compileShader("gltf_scene/shader.vert", wen::ShaderStage::eVertex);
    auto fragShader = interface->compileShader("gltf_scene/shader.frag", wen::ShaderStage::eFragment);
    shaderProgram_ = interface->createGraphicsShaderProgram();
    shaderProgram_->setVertexShader(vertShader);
    shaderProgram_->setFragmentShader(fragShader);

    renderPipeline_ = interface->createGraphicsRenderPipeline(renderer, shaderProgram_, "main subpass");
    renderPipeline_->setVertexInput(vertexInput_);
    renderPipeline_->setDescriptorSet(descriptorSet_);
    renderPipeline_->compile({
        .cullMode = wen::CullMode::eBack,
        .frontFace = wen::FrontFace::eCounterClockwise,
        .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
        .depthTestEnable = true,
    });

    vertexBuffer_ = interface->createVertexBuffer(sizeof(glm::vec3), scene_->vertices.size());
    vertexBuffer_->upload(scene_->vertices);
    indexBuffer_ = interface->createIndexBuffer(wen::IndexType::eUint32, scene_->indices.size());
    indexBuffer_->upload(scene_->indices);
}

void GLTFScene::update(float ts) {
    camera_->update(ts);
}

void GLTFScene::render() {
    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);

    renderer->getBindPoint(shaderProgram_);
    renderer->bindResources(renderPipeline_);
    renderer->setScissor(0, 0, w, h);
    renderer->setViewport(0, h, w, -h);
    renderer->bindVertexBuffer(vertexBuffer_);
    renderer->bindIndexBuffer(indexBuffer_);
    for (auto* node : scene_->nodesPtr()) {
        for (auto& primitive : node->getMesh()->primitives) {
            renderer->drawIndexed(primitive->indexCount, 1, primitive->data().firstIndex, primitive->data().firstVertex, 0);
        }
    }
}

void GLTFScene::imgui() {
    ImGui::Text("FrameRate: %f", ImGui::GetIO().Framerate);
}

void GLTFScene::destroy() {}