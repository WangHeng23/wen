#include <wen.hpp>

int main() {
    // 初始化引擎
    wen::initialize();

    // 设置引擎配置
    wen::settings->windowInfo = {"example", 900, 900};
    wen::settings->debug = true;
    wen::settings->appName = "example";
    wen::settings->setVsync(true);

    // 初始化渲染器
    auto& context = wen::initializeRenderer();

    // 创建接口
    auto interface = context.createInterface("./sandbox/example/resources");

    {
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

        auto renderer = interface->createRenderer(renderPass);

        auto vertShader = interface->createShader("vert.spv");
        auto fragShader = interface->createShader("frag.spv");
        auto shaderProgram = interface->createGraphicsShaderProgram();
        shaderProgram->setVertexShader(vertShader);
        shaderProgram->setFragmentShader(fragShader);

        auto renderPipeline = interface->createGraphicsRenderPipeline(renderer, shaderProgram, "main subpass");
        renderPipeline->compile({
            .topology = wen::Topology::eTriangleList,
            .polygonMode = wen::PolygonMode::eFill,
            .lineWidth = 2.0f,
            .cullMode = wen::CullMode::eNone,
            .dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor}
        });

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

            renderer->beginRender();
            renderer->getBindPoint(shaderProgram);
            renderer->bindResources(renderPipeline);
            renderer->setViewport(0, h, w, -h);
            renderer->setScissor(0, 0, w, h);
            renderer->draw(3, 1, 0, 0);
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