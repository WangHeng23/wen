#include "scenes/ray_tracing_1.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

void RayTracing::initialize() {
    auto renderPass = interface->createRenderPass();
    renderPass->addAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT, wen::AttachmentType::eColor);
    renderPass->addAttachment("depth buffer", wen::AttachmentType::eDepth);

    auto& subpass = renderPass->addSubpass("main subpass");
    subpass.setDepthStencilAttachment("depth buffer");
    subpass.setOutputAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT);

    renderPass->build();

    renderer = interface->createRenderer(std::move(renderPass));
    imguiLayer = interface->createImGuiLayer(renderer);

    createAccelerationStructure();

    shaderDescriptorSet_ = interface->createDescriptorSet();
    shaderDescriptorSet_->addDescriptors({
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment|wen::ShaderStage::eRaygen}, // info
        {1, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex|wen::ShaderStage::eRaygen}, // camera
    }).build();

    // info
    infoUniform_ = interface->createUniformBuffer(sizeof(Info));
    info_ = static_cast<Info*>(infoUniform_->getData());
    info_->scale = 1.0f;
    shaderDescriptorSet_->bindUniform(0, infoUniform_);
    // camera
    camera_ = std::make_unique<wen::Camera>();
    camera_->data.position = {0.0f, 0.0f, -3.5f};
    camera_->direction = {0.0f, 0.0f, 1.0f};
    camera_->upload();
    shaderDescriptorSet_->bindUniform(1, camera_->uniformBuffer);
    // point light
    pushConstants_ = interface->createPushConstants(
        wen::ShaderStage::eFragment | wen::ShaderStage::eClosestHit,
        {
            {"position", wen::ConstantType::eFloat3},
            {"color", wen::ConstantType::eFloat3},
            {"intensity", wen::ConstantType::eFloat},
        } 
    );
    pointLight_ = new PointLight();
    pointLight_->position = {2.0f, 2.0f, 2.0f};
    pointLight_->color = {1.0f, 1.0f, 1.0f};
    pointLight_->intensity = 5.0f;
    pushConstants_->pushConstant("position", &pointLight_->position);
    pushConstants_->pushConstant("color", &pointLight_->color);
    pushConstants_->pushConstant("intensity", &pointLight_->intensity);

    // ray tracing
    auto rgen = interface->compileShader("ray_tracing_1/raytrace.rgen", wen::ShaderStage::eRaygen);
    auto miss = interface->compileShader("ray_tracing_1/raytrace.rmiss", wen::ShaderStage::eMiss);
    auto shadow = interface->compileShader("ray_tracing_1/shadow.rmiss", wen::ShaderStage::eMiss);
    auto closest = interface->compileShader("ray_tracing_1/raytrace.rchit", wen::ShaderStage::eClosestHit);
    rayTracingDescriptorSet_ = interface->createDescriptorSet();
    rayTracingDescriptorSet_->addDescriptors({
        {0, wen::DescriptorType::eAccelerationStructure, wen::ShaderStage::eRaygen|wen::ShaderStage::eClosestHit},
        {1, wen::DescriptorType::eStorageImage, wen::ShaderStage::eRaygen},
        {2, wen::DescriptorType::eStorageBuffer, wen::ShaderStage::eClosestHit}
    }).build();
    rayTracingShaderProgram_ = interface->createRayTracingShaderProgram();
    rayTracingShaderProgram_->setRaygenShader(rgen);
    rayTracingShaderProgram_->setMissShader(miss);
    rayTracingShaderProgram_->setMissShader(shadow);
    rayTracingShaderProgram_->setHitGroup({closest, std::nullopt});
    rayTracingRenderPipeline_ = interface->createRayTracingRenderPipeline(rayTracingShaderProgram_);
    rayTracingRenderPipeline_->setDescriptorSet(rayTracingDescriptorSet_, 0);
    rayTracingRenderPipeline_->setDescriptorSet(shaderDescriptorSet_, 1);
    rayTracingRenderPipeline_->setPushConstants(pushConstants_);
    rayTracingRenderPipeline_->compile({
        .maxRayRecursionDepth = 2,
    });

    auto vs = interface->compileShader("ray_tracing_1/shader.vert", wen::ShaderStage::eVertex);
    auto fs = interface->compileShader("ray_tracing_1/shader.frag", wen::ShaderStage::eFragment);
    imageDescriptorSet_ = interface->createDescriptorSet();
    imageDescriptorSet_->addDescriptors({
        {0, wen::DescriptorType::eTexture, wen::ShaderStage::eFragment},
    }).build();
    graphicsShaderProgram_ = interface->createGraphicsShaderProgram();
    graphicsShaderProgram_->setVertexShader(vs);
    graphicsShaderProgram_->setFragmentShader(fs);
    graphicsRenderPipeline_ = interface->createGraphicsRenderPipeline(renderer, graphicsShaderProgram_, "main subpass");
    graphicsRenderPipeline_->setDescriptorSet(imageDescriptorSet_, 0);
    graphicsRenderPipeline_->setDescriptorSet(shaderDescriptorSet_, 1);
    graphicsRenderPipeline_->compile({
        .cullMode = wen::CullMode::eNone,
        .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
        .depthTestEnable = false,
    });

    auto [width, height] = wen::settings->windowSize;
    image_ = interface->createStorageImage(width, height, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eSampled);
    sampler_ = interface->createSampler({
        .minFilter = wen::SamplerFilter::eLinear,
    });
    rayTracingDescriptorSet_->bindAccelerationStructure(0, instance_);
    rayTracingDescriptorSet_->bindStorageImage(1, image_);
    rayTracingDescriptorSet_->bindStorageBuffer(2, instance_->instanceAddressBuffer());
    imageDescriptorSet_->bindTexture(0, image_, sampler_);

    // no ray tracing
    shaderProgram_ = interface->createGraphicsShaderProgram();
    shaderProgram_->setVertexShader(interface->compileShader("ray_tracing_1/raster.vert", wen::ShaderStage::eVertex));
    shaderProgram_->setFragmentShader(interface->compileShader("ray_tracing_1/raster.frag", wen::ShaderStage::eFragment));
    renderPipeline_ = interface->createGraphicsRenderPipeline(renderer, shaderProgram_, "main subpass");
    renderPipeline_->setVertexInput(
        interface->createVertexInput({
            {
                .binding = 0,
                .inputRate = wen::InputRate::eVertex,
                .formats = {
                    wen::VertexType::eFloat3, // position
                    wen::VertexType::eFloat3, // normal
                    wen::VertexType::eFloat3, // color
                }
            },
        })
    );
    renderPipeline_->setDescriptorSet(shaderDescriptorSet_);
    renderPipeline_->setPushConstants(pushConstants_);
    renderPipeline_->compile({
        .cullMode = wen::CullMode::eBack,
        .frontFace = wen::FrontFace::eCounterClockwise,
        .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
        .depthTestEnable = true,
    });
}

void RayTracing::createAccelerationStructure() {
    model1_ = interface->loadModel("mori_knob.obj");
    model2_ = interface->loadModel("cube.obj");
    vertexBuffer_ = interface->createVertexBuffer(sizeof(wen::Vertex), 4096000);
    indexBuffer_ = interface->createIndexBuffer(wen::IndexType::eUint32, 4096000);

    auto accelerationStructure = interface->createAccelerationStructure();
    accelerationStructure->addModel(model1_);
    accelerationStructure->addModel(model2_);
    accelerationStructure->build(false, false);

    instance_ = interface->createRayTracingInstance(false);
    instance_->addInstance({model1_}, glm::mat4(1.0f));
    instance_->build();

    model2_->upload(vertexBuffer_, indexBuffer_, model1_->upload(vertexBuffer_, indexBuffer_));
}

void RayTracing::update(float ts) {
    camera_->update(ts);
    static float time = 0;
    time += ImGui::GetIO().DeltaTime;
    pointLight_->position = glm::rotateY(glm::vec3(2.0f, pointLight_->position.y, 2.0f), time);
}

void RayTracing::render() {
    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    renderer->setClearColor(wen::SWAPCHAIN_IMAGE_ATTACHMENT, {{info_->clearColor.r, info_->clearColor.g, info_->clearColor.b, 1.0f}});

    if (isEnableRayTracing) {
        info_->windowSize = {w, h};
        renderer->getBindPoint(rayTracingShaderProgram_);
        renderer->bindResources(rayTracingRenderPipeline_);
        renderer->traceRays(rayTracingRenderPipeline_, w, h, 1);
        renderer->beginRenderPass();
        renderer->getBindPoint(graphicsShaderProgram_);
        renderer->bindResources(graphicsRenderPipeline_);
        renderer->setScissor(0, 0, w, h);
        renderer->setViewport(0, h, w, -h);
        renderer->draw(3, 2, 0, 0);
    } else {
        renderer->getBindPoint(shaderProgram_);
        renderer->bindResources(renderPipeline_);
        renderer->setScissor(0, 0, w, h);
        renderer->setViewport(0, h, w, -h);
        renderer->bindVertexBuffer(vertexBuffer_);
        renderer->bindIndexBuffer(indexBuffer_);
        renderer->drawModel(model1_, 1, 0);
    }
}

void RayTracing::imgui() {
    ImGui::Text("FrameRate: %f", ImGui::GetIO().Framerate);
    ImGui::Checkbox("Enabel Ray Tracing", &isEnableRayTracing);
    ImGui::SliderInt("Ray Tracing Scale", &info_->scale, 1, 5);
    ImGui::ColorEdit3("Clear Color", &info_->clearColor.r);
    ImGui::ColorEdit3("Light Color", &pointLight_->color.r);
    ImGui::SliderFloat("Light Intensity", &pointLight_->intensity, 0, 20);
    ImGui::SliderFloat("Light Height", &pointLight_->position.y, -5, 5);
    pushConstants_->pushConstant("position", &pointLight_->position);
    pushConstants_->pushConstant("color", &pointLight_->color);
    pushConstants_->pushConstant("intensity", &pointLight_->intensity);
}

void RayTracing::destroy() {}