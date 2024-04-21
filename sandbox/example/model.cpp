#include <wen.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

int main() {
    wen::initialize();

    // 设置引擎配置
    wen::settings->windowInfo = {"model", 900, 900};
    wen::settings->debug = true;
    wen::settings->appName = "model";
    wen::settings->setVsync(true);
    wen::settings->defaultFont = "./sandbox/resources/fonts/JetBrainsMonoNLNerdFontMono-Bold.ttf";
    wen::settings->chineseFont = "./sandbox/resources/fonts/SourceHanSansCN-Normal.ttf";

    auto& context = wen::initializeRenderer();

    auto interface = context.createInterface("./sandbox/example/resources");

    auto renderPass = interface->createRenderPass();
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

    {
        auto renderer = interface->createRenderer(std::move(renderPass));

        auto imguiLayer = interface->createImGuiLayer(renderer);

        auto vertShader = interface->compileShader("model.vert", wen::ShaderStage::eVertex);
        auto fragShader = interface->compileShader("model.frag", wen::ShaderStage::eFragment);
        auto shaderProgram = interface->createGraphicsShaderProgram();
        shaderProgram->setVertexShader(vertShader);
        shaderProgram->setFragmentShader(fragShader);

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

        auto descriptorSet = interface->createDescriptorSet();
        descriptorSet->addDescriptors({
            {0, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex}, // camera
        }).build();

        auto renderPipeline = interface->createGraphicsRenderPipeline(renderer, shaderProgram, "main subpass");
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

        // camera
        auto camera = std::make_unique<wen::Camera>();
        camera->data.position = {0.0f, 0.0f, -3.0f};
        camera->direction = {0.0f, 0.0f, 1.0f};
        camera->upload();
        descriptorSet->bindUniform(0, camera->uniform);

        // model
        auto model = interface->loadModel("dragon.obj");
        auto vertexBuffer = interface->createVertexBuffer(sizeof(wen::Vertex), model->vertexCount);
        auto indexBuffer = interface->createIndexBuffer(wen::IndexType::eUint32, model->indexCount);
        model->upload(vertexBuffer, indexBuffer);

        while (!wen::shouldClose()) {
            wen::pollEvents();

            camera->update(ImGui::GetIO().DeltaTime);

            static auto start = std::chrono::high_resolution_clock::now();
            auto current = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration<float, std::chrono::seconds::period>(current - start).count();
            float scale = std::cos(time * 3.5f);
            float color = (scale + 2.0f) / 3.0f;

            renderer->setClearColor(wen::SWAPCHAIN_IMAGE_ATTACHMENT, {{(1 + color) / 2.0f, 1 - color, color, 1.0f}});

            auto [width, height] = wen::settings->windowSize;
            auto w = static_cast<float>(width), h = static_cast<float>(height);

            renderer->beginRender();
            renderer->getBindPoint(shaderProgram);
            renderer->bindResources(renderPipeline);
            renderer->setViewport(0, h, w, -h);
            renderer->setScissor(0, 0, w, h);
            renderer->bindVertexBuffers({vertexBuffer});
            renderer->bindIndexBuffer(indexBuffer);
            renderer->drawModel(model, 1, 0);

            imguiLayer->begin();
            ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
            ImGui::Text("vertex count: %d", model->vertexCount);
            ImGui::Text("index count: %d", model->indexCount);
            ImGui::Text("triangle count: %d", model->indexCount / 3);
            imguiLayer->end();

            renderer->endRender();
        }
        renderer->waitIdle();
    }

    wen::destroyRenderer();

    wen::destroy();

    return 0;
}