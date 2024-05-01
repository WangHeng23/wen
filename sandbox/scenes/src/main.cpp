#include "scenes/model_scene.hpp"
#include "scenes/shader_toy.hpp"
#include "scenes/pbr_scene.hpp"
#include "scenes/ray_marching.hpp"
#include "scenes/model_manager.hpp"
#include "scenes/ray_tracing_1.hpp"

int main() {
    wen::initialize();

    // 设置引擎配置
    wen::settings->windowInfo = {"scene", 1600, 900};
    wen::settings->debug = true;
    wen::settings->appName = "scene";
    wen::settings->setVsync(true);
    wen::settings->defaultFont = "./sandbox/resources/fonts/JetBrainsMonoNLNerdFontMono-Bold.ttf";
    wen::settings->chineseFont = "./sandbox/resources/fonts/SourceHanSansCN-Normal.ttf";
    wen::settings->isEnableRayTracing = true;
    wen::settings->deviceRequestedExtensions.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    wen::logger->setLevel(wen::Logger::Level::info);

    auto& context = wen::initializeRenderer();
    wen::settings->setSampleCount(wen::SampleCount::e1);
    auto interface = context.createInterface("./sandbox/scenes/resources");

    auto sceneManager = new SceneManager(interface);
    // sceneManager->setScene<ModelScene>();
    // sceneManager->setScene<ShaderToy>();
    // sceneManager->setScene<PBRScene>();
    // sceneManager->setScene<RayMarching>();
    // sceneManager->setScene<ModelManager>();
    sceneManager->setScene<RayTracing>();
    while (!wen::shouldClose()) {
        wen::pollEvents();
        sceneManager->update();
        sceneManager->render();
    }
    delete sceneManager;

    wen::destroyRenderer();
    wen::destroy();

    return 0;
}