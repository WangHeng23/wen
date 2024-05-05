#include "scenes/ray_tracing_1.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include <functional>

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
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eFragment | wen::ShaderStage::eMiss}, // info
        {1, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex | wen::ShaderStage::eRaygen}, // camera
    }).build();

    // info
    infoUniform_ = interface->createUniformBuffer(sizeof(Info));
    info_ = static_cast<Info*>(infoUniform_->getData());
    info_->clearColor = glm::vec3(0.8f, 0.8f, 0.9f);
    shaderDescriptorSet_->bindUniform(0, infoUniform_);
    // camera
    camera_ = std::make_unique<wen::Camera>();
    camera_->data.position = {0.0f, 1.0f, -3.5f};
    camera_->direction = {0.0f, 0.0f, 1.0f};
    camera_->upload();
    shaderDescriptorSet_->bindUniform(1, camera_->uniformBuffer);
    // point light
    pushConstants_ = interface->createPushConstants(
        wen::ShaderStage::eFragment | wen::ShaderStage::eRaygen | wen::ShaderStage::eClosestHit,
        {
            {"position", wen::ConstantType::eFloat3},
            {"color", wen::ConstantType::eFloat3},
            {"intensity", wen::ConstantType::eFloat},
            {"sample count", wen::ConstantType::eInt32},
            {"frame", wen::ConstantType::eInt32},
        } 
    );
    pointLightPosition_ = glm::vec3(2.0f, 2.0f, 2.0f);

    // ray tracing
    auto rgen = interface->compileShader("ray_tracing_1/raytrace.rgen", wen::ShaderStage::eRaygen);
    auto miss = interface->compileShader("ray_tracing_1/raytrace.miss", wen::ShaderStage::eMiss);
    auto closest = interface->compileShader("ray_tracing_1/raytrace.rchit", wen::ShaderStage::eClosestHit);
    auto shadow = interface->compileShader("ray_tracing_1/shadow.miss", wen::ShaderStage::eMiss);
    rayTracingDescriptorSet_ = interface->createDescriptorSet();
    rayTracingDescriptorSet_->addDescriptors({
        {0, wen::DescriptorType::eAccelerationStructure, wen::ShaderStage::eRaygen|wen::ShaderStage::eClosestHit},
        {1, wen::DescriptorType::eStorageImage, wen::ShaderStage::eRaygen},
        {2, wen::DescriptorType::eStorageBuffer, wen::ShaderStage::eClosestHit}
    }).build();
    rayTracingShaderProgram_ = interface->createRayTracingShaderProgram();
    rayTracingShaderProgram_->setRaygenShader(rgen);
    rayTracingShaderProgram_->setMissShader(miss);
    rayTracingShaderProgram_->setHitGroup({closest, std::nullopt});
    rayTracingShaderProgram_->setMissShader(shadow);
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
    // model1_ = interface->loadModel("mori_knob.obj");
    // model2_ = interface->loadModel("mori_knob.obj");
    model1_ = interface->loadModel("ray_tracing/wuson.obj");
    model2_ = interface->loadModel("ray_tracing/plane.obj");
    model3_ = interface->loadModel("dragon.obj");
    vertexBuffer_ = interface->createVertexBuffer(sizeof(wen::Vertex), 4096000);
    indexBuffer_ = interface->createIndexBuffer(wen::IndexType::eUint32, 4096000);

    auto accelerationStructure = interface->createAccelerationStructure();
    accelerationStructure->addModel(model1_);
    accelerationStructure->addModel(model2_);
    accelerationStructure->addModel(model3_);
    accelerationStructure->build(false, false);
    accelerationStructure.reset();

    instance_ = interface->createRayTracingInstance();
    instance_->addModel(0, {model1_, model2_}, glm::mat4(1.0f));
    std::random_device device;
    std::mt19937 generator(device());
    std::normal_distribution<float> distribution(0, 1);
    std::normal_distribution<float> scaleDistribution(0.3f, 0.2f);
    std::uniform_real_distribution<float> colorDistribution(0, 1);
    for (int i = 0; i < 500; i++) {
        auto position = glm::vec3(distribution(generator) * 2, distribution(generator) * 2, distribution(generator) * 2);
        auto rotateAxis = glm::vec3(distribution(generator), distribution(generator), distribution(generator));
        auto rotateAngle = distribution(generator) * 3.1415926535f;
        auto scale = scaleDistribution(generator);
        transformInfos[1].push_back({position, rotateAxis, rotateAngle, scale});
        auto transform =
            glm::translate(position) *
            glm::rotate(rotateAngle, rotateAxis) *
            glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        instance_->addModel(1, {model3_}, transform);
    }
    instance_->addModel(2, {model1_}, glm::mat4(1.0f));
    transformInfos[2].push_back({
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        0.0f,
        1.0f
    });
    instance_->build(true);

    auto offset1 = model1_->upload(vertexBuffer_, indexBuffer_);
    auto offset2 = model2_->upload(vertexBuffer_, indexBuffer_, offset1);
    auto offset3 = model3_->upload(vertexBuffer_, indexBuffer_, offset2);
}

void RayTracing::update(float ts) {
    camera_->update(ts);
    static float time = 0;
    time += ImGui::GetIO().DeltaTime;
    pointLightPosition_ = glm::rotateY(glm::vec3(2.0f, pointLightPosition_.y, 2.0f), time);

    if (isEnableRayTracing) {
        pushConstants_->pushConstant("frame", &frame_);
        frame_++;
        if (camera_->isCursorLocked) {
            frame_ = 0;
        }

        instance_->allow_update ? instance_->update(1, [&](uint32_t index, auto updateTransform) {
            auto [position, rotateAxis, rotateAngle, scale] = transformInfos.at(1)[index];
            rotateAngle += time;
            scale *= (cos(time * 1.8f + rotateAngle) + 3.0f) / 3.0f;
            // 开普勒第三定律
            auto distance3 = glm::pow(glm::length(position), 3.0f);
            auto k = 2.0f / distance3;
            auto w = glm::sqrt(k);
            position = glm::rotate(w * time, rotateAxis) * glm::vec4(position, 1);
            auto transform =
                glm::translate(position) *
                glm::rotate(rotateAngle, rotateAxis) *
                glm::scale(glm::mat4(1.0f), glm::vec3(scale));
            updateTransform(transform);
        }) : void();
    
        instance_->allow_update ? instance_->update(2, [&](uint32_t index, auto updateTransform) {
            auto [position, rotateAxis, rotateAngle, scale] = transformInfos.at(2)[index];
            auto transform =
                glm::translate(position) *
                glm::rotate(rotateAngle, rotateAxis) *
                glm::scale(glm::mat4(1.0f), glm::vec3(scale));
            updateTransform(transform);
        }) : void();
    }
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
        renderer->drawModel(model2_, 1, 0);
    }
}

void RayTracing::imgui() {
    bool changed = false;

    ImGui::Text("FrameRate: %f", ImGui::GetIO().Framerate);
    ImGui::Checkbox("Enabel Ray Tracing", &isEnableRayTracing);
    ImGui::ColorEdit3("Clear Color", &info_->clearColor.r);

    ImGui::SeparatorText("Light");
    changed |= ImGui::SliderFloat("Light Height", &pointLightPosition_.y, -1, 6);
    pushConstants_->pushConstant("position", &pointLightPosition_);

    static glm::vec3 color = {1.0f, 1.0f, 1.0f};
    changed |= ImGui::ColorEdit3("Light Color", &color.r);
    pushConstants_->pushConstant("color", &color);

    static float intensity = 5.0f;
    changed |= ImGui::SliderFloat("Light Intensity", &intensity, 0, 30);
    pushConstants_->pushConstant("intensity", &intensity);

    static int sampleCount = 1;
    changed |= ImGui::SliderInt("Sample Count", &sampleCount, 1, 9);
    pushConstants_->pushConstant("sample count", &sampleCount);

    if (changed) {
        frame_ = 0;
    }
    ImGui::Text("Frame Index: %d", frame_);
}

void RayTracing::destroy() {}