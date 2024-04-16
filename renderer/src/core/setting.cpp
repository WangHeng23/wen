#include "core/setting.hpp"
#include "core/logger.hpp"

namespace wen {

std::shared_ptr<Settings> settings = nullptr;

void Settings::setWindowSize(uint32_t width, uint32_t height) {
    windowSize = {width, height};
}

void Settings::setVsync(bool vsync) {
    this->vsync = vsync;
    WEN_INFO("Vsync is {}", vsync ? "enabled" : "disabled")
}

} // namespace wen