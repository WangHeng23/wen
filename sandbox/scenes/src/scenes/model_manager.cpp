#include "scenes/model_manager.hpp"

void ModelManager::initialize() {
    auto renderPass = interface->createRenderPass();
    renderPass->addAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT, wen::AttachmentType::eColor);
    renderPass->addAttachment("depth buffer", wen::AttachmentType::eDepth);

    auto& subpass = renderPass->addSubpass("main subpass");
    subpass.setDepthStencilAttachment("depth buffer");
    subpass.setOutputAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT);

    renderPass->build();

    renderer = interface->createRenderer(std::move(renderPass));
    imguiLayer = interface->createImGuiLayer(renderer);

    auto vertexShader = interface->compileShader("model_manager/shader.vert", wen::ShaderStage::eVertex);
    auto fragmentShader = interface->compileShader("model_manager/shader.frag", wen::ShaderStage::eFragment);
    shaderProgram_ = interface->createGraphicsShaderProgram();
    shaderProgram_->setVertexShader(vertexShader);
    shaderProgram_->setFragmentShader(fragmentShader);

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
                wen::VertexType::eFloat   // scale
            }
        }
    });

    auto descriptorSet = interface->createDescriptorSet();
    descriptorSet->addDescriptors({
        {0, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex}
    }).build();

    vertexBuffer_ = interface->createVertexBuffer(sizeof(wen::VertexType), 4096000);
    indexBuffer_ = interface->createIndexBuffer(wen::IndexType::eUint32, 4096000);

    camera_ = std::make_unique<wen::Camera>();
    camera_->data.position = {0.0f, 0.0f, -3.0f};
    camera_->direction = {0.0f, 0.0f, 1.0f};
    camera_->upload();
    descriptorSet->bindUniform(0, camera_->uniformBuffer);

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

void ModelManager::update(float ts) {
    camera_->update(ts);
}

void ModelManager::render() {
    renderer->setClearColor(wen::SWAPCHAIN_IMAGE_ATTACHMENT, {{0.3f, 0.8f, 1.0f, 1.0f}});
    auto [width, height] = wen::settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    renderer->getBindPoint(shaderProgram_);
    renderer->bindResources(renderPipeline_);
    renderer->setViewport(0, h, w, -h);
    renderer->setScissor(0, 0, w, h);
    renderer->bindVertexBuffer(vertexBuffer_);
    renderer->bindIndexBuffer(indexBuffer_);
    for (auto& [filename, info] : models_) {
        info.vertexBuffer->upload(info.innerInfos);
        renderer->bindVertexBuffer(info.vertexBuffer, 1);
        for (auto& [meshName, visable] : info.meshNameVisible) {
            if (!visable) {
                continue;
            }
            renderer->drawMesh(info.model->meshes().at(meshName), info.innerInfos.size(), 0);
        }
    }
}

void ModelManager::imgui() {
    ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
    ImGui::Separator();

    static const char* filenames[] = {"mori_knob.obj", "dragon.obj", "bunny.obj", "teapot.obj", "Red.obj", "sportsCar.obj"};
    static int idx = 0;
    ImGui::Combo("模型文件名", &idx, filenames, IM_ARRAYSIZE(filenames));

    static char filename[1024];
    strcpy_s(filename, filenames[idx]);
    static char name[1024] = {0};
    static wen::Offset modelOffset = {0, 0};

    ImGui::InputText("实例名", name, 1024);
    if (ImGui::Button("加载模型")) {
        if (models_.find(filename) == models_.end()) {
            auto model = interface->loadModel(filename);
            modelOffset = model->upload(vertexBuffer_, indexBuffer_, modelOffset);
            models_.insert(std::make_pair(
                filename,
                ModelInfo {
                    model,
                    interface->createVertexBuffer(sizeof(ModelInfo::InnerInfo), 512),
                    {},
                    {}
                }
            ));
            WEN_INFO("加载模型：{}", filename)
        }

        if (querys.find(name) == querys.end()) {
            querys.insert(std::make_pair(name, std::make_pair(filename, models_[filename].innerInfos.size())));
            models_[filename].innerInfos.push_back({
                .offset = {0.0f, 0.0f, 0.0f},
                .scale = 1.0f
            });
        } else {
            WEN_ERROR("已存在名为\"{}\"的实例", name)
        }
    }

    ImGui::SeparatorText("已创建的实例模型");
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, -1.0f});
    static int selectedNumber = -1;
    int i = 0;
    static std::string selected;
    for (auto& [name, info] : querys) {
        if (ImGui::RadioButton(name.c_str(), &selectedNumber, i)) {
            selected = name;
        }
        i += 1;
    }
    ImGui::PopStyleVar();

    ImGui::SeparatorText("选择的模型实例属性");
    if (!selected.empty()) {
        auto& query = querys[selected];
        auto& modelInfo = models_[query.first];
        auto& innerInfo = modelInfo.innerInfos[query.second];
        ImGui::SliderFloat3("offset", &innerInfo.offset.x, -10, 10);
        ImGui::SliderFloat("scale", &innerInfo.scale, 0.0001, 10);
        auto model = modelInfo.model;
        ImGui::Text("vertex count: %d", model->vertexCount);
        ImGui::Text("index count: %d", model->indexCount);
        ImGui::Text("triangle count: %d", model->indexCount / 3);
    }

    ImGui::SeparatorText("选择的模型网格名称");
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, -1.0f});
    if (!selected.empty()) {
        auto& info = models_[querys[selected].first];
        std::vector<std::string> meshNames;
        for (auto& [name, mesh] : info.model->meshes()) {
            meshNames.push_back(name);
        }
        for (auto& meshName : meshNames) {
            ImGui::Checkbox(meshName.c_str(), &info.meshNameVisible[meshName]);
        }
        ImGui::Separator();
    }
    ImGui::PopStyleVar();
}

void ModelManager::destroy() {
    camera_.reset();
    shaderProgram_.reset();
    renderPipeline_.reset();
    vertexBuffer_.reset();
    indexBuffer_.reset();
}