#pragma once

#include "base/window.hpp"
#include <vulkan/vulkan.hpp>

namespace wen {

struct WindowSize {
    uint32_t width, height;
};

class Settings {
public:
    Settings() = default;

    void setWindowSize(uint32_t width, uint32_t height);
    void setVsync(bool vsync);

    Window::Info windowInfo;
    bool debug = false;
    std::string appName = "app name";
    uint32_t appVersion = 1;
    std::string engineName = "engine name";
    uint32_t engineVersion = 1;
    std::vector<std::string> deviceRequestedExtensions = {};

    WindowSize windowSize;
    std::optional<vk::SurfaceFormatKHR> desiredFormat = std::nullopt;
    std::optional<vk::PresentModeKHR> desiredPresentMode = std::nullopt;
    bool vsync = false;

    uint32_t maxFramesInFlight = 2;
    uint32_t currentInFlight = 0;

    // ImGui
    std::string defaultFont = "";
    std::string chineseFont = "";
    float fontSize = 18.0f;
};

extern std::shared_ptr<Settings> settings;

const std::string SWAPCHAIN_IMAGE_ATTACHMENT = "SWAPCHAIN_IMAGE_ATTACHMENT";
const std::string EXTERNAL_SUBPASS = "EXTERNAL_SUBPASS";

} // namespace wen