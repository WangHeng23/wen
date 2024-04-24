#include "scenes/model_scene.hpp"
#include "scenes/shader_toy.hpp"
#include "scenes/pbr_scene.hpp"

int main() {
    wen::initialize();

    // 设置引擎配置
    wen::settings->windowInfo = {"scene", 1600, 900};
    wen::settings->debug = true;
    wen::settings->appName = "scene";
    wen::settings->setVsync(true);
    wen::settings->defaultFont = "./sandbox/resources/fonts/JetBrainsMonoNLNerdFontMono-Bold.ttf";
    wen::settings->chineseFont = "./sandbox/resources/fonts/SourceHanSansCN-Normal.ttf";
    wen::logger->setLevel(wen::Logger::Level::info);

    auto& context = wen::initializeRenderer();
    wen::settings->setSampleCount(wen::SampleCount::e64);
    auto interface = context.createInterface("./sandbox/scenes/resources");

    auto sceneManager = new SceneManager(interface);
    // sceneManager->setScene<ModelScene>();
    // sceneManager->setScene<ShaderToy>();
    sceneManager->setScene<PBRScene>();
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