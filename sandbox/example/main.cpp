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

    // 初始化渲染器后，才进行设置采样数
    wen::settings->setSampleCount(wen::SampleCount::e64);

    // 创建接口
    auto interface = context.createInterface("./sandbox/example/resources");

    // 创建渲染流程
    auto renderPass = interface->createRenderPass();
    // 创建渲染流程后，会添加默认的交换链图像附件，不用像下面这样手动添加
    // 这里修改了代码，需要手动添加
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

    {
        auto renderer = interface->createRenderer(std::move(renderPass));

        auto imguiLayer = interface->createImGuiLayer(renderer);

        auto vertShader = interface->compileShader("shader.vert", wen::ShaderStage::eVertex);
        auto fragShader = interface->compileShader("shader.frag", wen::ShaderStage::eFragment);
        auto shaderProgram = interface->createGraphicsShaderProgram();
        shaderProgram->setVertexShader(vertShader);
        shaderProgram->setFragmentShader(fragShader);

        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 uv;
        };

        const std::vector<Vertex> vertices = {
            {{ 0.5f,  0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f, 0.2f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, 0.2f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.3f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        };

        const std::vector<glm::vec3> offsets = {
            { 0.7f,  0.7f, 4.0}, // 第一象限
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
                    wen::VertexType::eFloat3, // position
                    wen::VertexType::eFloat3, // color
                    wen::VertexType::eFloat2  // uv
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

        auto descriptorSet = interface->createDescriptorSet();
        descriptorSet->addDescriptors({
            {0, wen::DescriptorType::eUniform, wen::ShaderStage::eVertex}, // camera
            {1, wen::DescriptorType::eTexture, wen::ShaderStage::eFragment} // texture
        }).build();

        auto pushConstans = interface->createPushConstants(
            wen::ShaderStage::eVertex,
            {
                {"pad", wen::ConstantType::eFloat3}, // test
                {"scaler", wen::ConstantType::eFloat},
                {"time", wen::ConstantType::eFloat},
            }
        );

        auto renderPipeline = interface->createGraphicsRenderPipeline(renderer, shaderProgram, "main subpass");
        renderPipeline->setVertexInput(vertexInput);
        renderPipeline->setDescriptorSet(descriptorSet);
        renderPipeline->setPushConstants(pushConstans);
        renderPipeline->compile({
            .topology = wen::Topology::eTriangleList,
            .polygonMode = wen::PolygonMode::eFill,
            .lineWidth = 2.0f,
            .cullMode = wen::CullMode::eNone,
            .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
            .depthTestEnable = true,
        });

        auto camera = std::make_unique<wen::Camera>();
        camera->data.position = {0.0f, 0.0f, 6.0f};
        camera->direction = {0.0f, 0.0f, -1.0f};
        camera->upload();
        auto texture = interface->createTexture("texture.jpg", 4);
        auto sampler = interface->createSampler({
            .magFilter = wen::SamplerFilter::eNearest,
            .minFilter = wen::SamplerFilter::eLinear,
            .addressModeU = wen::SamplerAddressMode::eMirroredRepeat,
            .addressModeV = wen::SamplerAddressMode::eClampToBorder,
            .addressModeW = wen::SamplerAddressMode::eRepeat,
            .maxAnisotropy = 16,
            .borderColor = wen::SamplerBorderColor::eFloatOpaqueBlack,
            .mipmapMode = wen::SamplerFilter::eLinear,
            .mipLevels = texture->getMipLevels()
        });
        descriptorSet->bindUniform(0, camera->uniform);
        descriptorSet->bindTexture(1, texture, sampler);

        // 主循环
        while (!wen::shouldClose()) {
            wen::pollEvents();

            camera->update(ImGui::GetIO().DeltaTime);

            static auto start = std::chrono::high_resolution_clock::now();
            auto current = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration<float, std::chrono::seconds::period>(current - start).count();
            float scale = std::cos(time * 2.5f);
            float color = (scale + 2.0f) / 3.0f;

            pushConstans->pushConstant("scaler", &scale);
            pushConstans->pushConstant("time", &time);

            renderer->setClearColor(wen::SWAPCHAIN_IMAGE_ATTACHMENT, {{(1 + color) / 2.0f, 1 - color, color, 1.0f}});

            auto [width, height] = wen::settings->windowSize;
            auto w = static_cast<float>(width), h = static_cast<float>(height);

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
            // ImGui::ShowDemoWindow();
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