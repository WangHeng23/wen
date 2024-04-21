#include "core/setting.hpp"
#include "utils/utils.hpp"
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

void Settings::setSampleCount(SampleCount count) {
    static SampleCount maxSampleCount = getMaxUsableSampleCount();
    if (static_cast<int>(count) > static_cast<int>(maxSampleCount)) {
        WEN_WARN("max sample count is {}", static_cast<uint32_t>(maxSampleCount))
        count = maxSampleCount;
    }
    msaaSamples = count;
}

} // namespace wen