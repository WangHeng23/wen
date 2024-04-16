#include "manager.hpp"
#include "core/logger.hpp"
#include "core/setting.hpp"

namespace wen {

Context* manager = nullptr;

void initialize() {
    logger = new Logger(wen::Logger::Level::trace);
    settings = std::make_shared<Settings>();
}

Context& initializeRenderer() {
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
}

void destroy() {
    settings.reset();
    settings = nullptr;
    delete logger;
}

} // namespace wen