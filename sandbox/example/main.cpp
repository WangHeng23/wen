#include <wen.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

int main() {
    // 初始化引擎
    wen::initialize();

    // 设置引擎配置
    wen::settings->windowInfo = {"example", 900, 900};
    wen::settings->debug = true;
    wen::settings->appName = "example";
    wen::settings->setVsync(true);
    wen::settings->defaultFont = "./sandbox/resources/fonts/JetBrainsMonoNLNerdFontMono-Bold.ttf";
    wen::settings->chineseFont = "./sandbox/resources/fonts/SourceHanSansCN-Normal.ttf";

    // 初始化渲染器
    auto& context = wen::initializeRenderer();

    // 创建接口
    auto interface = context.createInterface("./sandbox/example/resources");

    // 创建渲染流程
    auto renderPass = interface->createRenderPass();
    // 创建渲染流程后，会添加默认的交换链图像附件，不用像下面这样手动添加
    // renderPass->addAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT, wen::AttachmentType::eColor);
    
    auto& subpass = renderPass->addSubpass("main subpass");
    subpass.setOutputAttachment(wen::SWAPCHAIN_IMAGE_ATTACHMENT);
    
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

        auto vertShader = interface->compileShader("shader.vert", wen::ShaderStage::eVertex);
        auto fragShader = interface->compileShader("shader.frag", wen::ShaderStage::eFragment);
        auto shaderProgram = interface->createGraphicsShaderProgram();
        shaderProgram->setVertexShader(vertShader);
        shaderProgram->setFragmentShader(fragShader);

        struct Vertex {
            glm::vec2 position;
            glm::vec3 color;
        };

        const std::vector<Vertex> vertices = {
            {{ 0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
            {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
        };

        const std::vector<glm::vec3> offsets = {
            { 0.7f,  0.7f, 1.0}, // 第一象限
            {-0.7f,  0.7f, 2.0}, // 第二象限
            { 0.7f, -0.7f, 2.0}, // 第四象限
            {-0.7f, -0.7f, 3.0}  // 第三象限
        };
        const std::vector<uint16_t> indices = {0, 1, 2, 1, 2, 3};

        auto vertexInput = interface->createVertexInput({
            {
                .binding = 0,
                .inputRate = wen::InputRate::eVertex,
                .formats = {
                    wen::VertexType::eFloat2, // position
                    wen::VertexType::eFloat3  // color
                }
            },
            {
                .binding = 1,
                .inputRate = wen::InputRate::eInstance,
                .formats = {
                    wen::VertexType::eFloat3  // offset
                }
            }
        });

        auto vertexBuffer = interface->createVertexBuffer(sizeof(Vertex), vertices.size());
        vertexBuffer->upload(vertices);
        auto offsetBuffer = interface->createVertexBuffer(sizeof(glm::vec3), offsets.size());
        offsetBuffer->upload(offsets);
        auto indexBuffer = interface->createIndexBuffer(wen::IndexType::eUint16, indices.size());
        indexBuffer->upload(indices);

        struct UniformBufferObject {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        auto descriptorSet = interface->createDescriptorSet();
        descriptorSet->addDescriptors({
            {0, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex}, // mvp
        }).build();

        auto renderPipeline = interface->createGraphicsRenderPipeline(renderer, shaderProgram, "main subpass");
        renderPipeline->setVertexInput(vertexInput);
        renderPipeline->setDescriptorSet(descriptorSet);
        renderPipeline->compile({
            .topology = wen::Topology::eTriangleList,
            .polygonMode = wen::PolygonMode::eFill,
            .lineWidth = 2.0f,
            .cullMode = wen::CullMode::eNone,
            .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor}
        });

        auto uniform = interface->createUniformBuffer(sizeof(UniformBufferObject));
        descriptorSet->bindUniform(0, uniform);

        // 主循环
        while (!wen::shouldClose()) {
            wen::pollEvents();

            static auto start = std::chrono::high_resolution_clock::now();
            auto current = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration<float, std::chrono::seconds::period>(current - start).count();
            float scale = std::cos(time * 2.0f);
            float color = (scale + 2.0f) / 3.0f;

            renderer->setClearColor(wen::SWAPCHAIN_IMAGE_ATTACHMENT, {{(1 + color) / 2.0f, 1 - color, color, 1.0f}});

            auto [width, height] = wen::settings->windowSize;
            auto w = static_cast<float>(width), h = static_cast<float>(height);

            auto data = (UniformBufferObject*)uniform->getData();
            data->model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            data->view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            data->proj = glm::perspective(glm::radians(45.0f), w / h, 0.1f, 10.0f);

            renderer->beginRender();
            renderer->getBindPoint(shaderProgram);
            renderer->bindResources(renderPipeline);
            renderer->setViewport(0, h, w, -h);
            renderer->setScissor(0, 0, w, h);
            renderer->bindVertexBuffers({vertexBuffer, offsetBuffer});
            renderer->bindIndexBuffer(indexBuffer);
            // renderer->draw(3, 1, 0, 0);
            renderer->drawIndexed(indices.size(), offsets.size(), 0, 0, 0);

            // ImGui
            imguiLayer->begin();
            ImGui::Text("中文字体");
            ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
            ImGui::ShowDemoWindow();
            imguiLayer->end();

            renderer->endRender();
        }
        renderer->waitIdle();
    }

    // 销毁渲染器
    wen::destroyRenderer();

    // 销毁引擎
    wen::destroy();

    return 0;
}