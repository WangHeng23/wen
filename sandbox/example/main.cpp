#include <wen.hpp>

int main() {
    // 初始化引擎
    wen::initialize();

    // 设置引擎配置
    wen::settings->windowInfo = {"example", 900, 900};
    wen::settings->debug = true;
    wen::settings->appName = "example";

    // 初始化渲染器
    auto& context = wen::initializeRenderer();

    // 主循环
    while (!wen::shouldClose()) {
        wen::pollEvents();
    }

    // 销毁渲染器
    wen::destroyRenderer();

    // 销毁引擎
    wen::destroy();

    return 0;
}