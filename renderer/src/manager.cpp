#include "manager.hpp"
#include "core/logger.hpp"
#include "core/setting.hpp"

namespace wen {

Context* manager = nullptr;

void initialize() {
    settings = std::make_shared<Settings>();
}

Context& initializeRenderer() {
    logger = new Logger(wen::Logger::Level::trace);
    window = std::make_unique<Window>(settings->windowInfo);
    Context::init();
    Context::instance().initialize();
    return Context::instance();
}

bool shouldClose() {
    return window->shouldClose();
}

void pollEvents() {
    window->pollEvents();
}

void destroyRenderer() {
    Context::instance().destroy();
    Context::quit();
    window.reset();
    window = nullptr;
    delete logger;
}

void destroy() {
    settings.reset();
    settings = nullptr;
}

} // namespace wen