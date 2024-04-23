#include "ray_tracing.hpp"

int main() {
    wen::initialize();

    wen::settings->windowInfo = {"RayTracing", 1600, 900};
    wen::settings->debug = true;
    wen::settings->appName = "RayTracing";
    wen::settings->defaultFont = "./sandbox/resources/fonts/JetBrainsMonoNLNerdFontMono-Bold.ttf";
    wen::logger->setLevel(wen::Logger::Level::info);

    auto app = new Application();
    app->pushLayer<RayTracing>();
    app->init();
    app->run();
    delete app;

    wen::destroy();

    return 0;
}