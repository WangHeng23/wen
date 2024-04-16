#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class Swapchain final {
public:
    Swapchain();
    ~Swapchain();

public:
    vk::SwapchainKHR swapchain;
    vk::SurfaceFormatKHR format;
    vk::PresentModeKHR presentMode;
    uint32_t imageCount;
    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;

private:
    struct Infos {
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<vk::SurfaceFormatKHR> surfaceFormats;
        std::vector<vk::PresentModeKHR> presentModes;
    };
};

} // namespace wen