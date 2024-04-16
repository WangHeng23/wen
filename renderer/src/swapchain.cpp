#include "swapchain.hpp"
#include "core/setting.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

Swapchain::Swapchain() {
    Infos infos = {
        manager->device->physicalDevice.getSurfaceCapabilitiesKHR(manager->surface),
        manager->device->physicalDevice.getSurfaceFormatsKHR(manager->surface),
        manager->device->physicalDevice.getSurfacePresentModesKHR(manager->surface)
    };

    format = infos.surfaceFormats[0];
    presentMode = infos.presentModes[0];

    vk::SurfaceFormatKHR desiredFormat;
    if (settings->desiredFormat.has_value()) {
        desiredFormat = settings->desiredFormat.value();
    } else {
        desiredFormat = {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    }
    for (auto format : infos.surfaceFormats) {
        if (format.format == desiredFormat.format && format.colorSpace == desiredFormat.colorSpace) {
            this->format = format;
            break;
        }
    }

    vk::PresentModeKHR desiredPresentMode;
    if (settings->desiredPresentMode.has_value()) {
        desiredPresentMode = settings->desiredPresentMode.value();
    } else {
        if (settings->vsync) {
            desiredPresentMode = vk::PresentModeKHR::eMailbox;
        } else {
            desiredPresentMode = vk::PresentModeKHR::eImmediate;
        }
    }
    for (auto presentMode : infos.presentModes) {
        if (presentMode == desiredPresentMode) {
            this->presentMode = presentMode;
            break;
        }
    }
    
    imageCount = std::clamp<uint32_t>(presentMode == vk::PresentModeKHR::eMailbox ? 4 : 3,
                                      infos.surfaceCapabilities.minImageCount,
                                      infos.surfaceCapabilities.maxImageCount);

    uint32_t width = std::clamp<uint32_t>(infos.surfaceCapabilities.currentExtent.width,
                                          infos.surfaceCapabilities.minImageExtent.width,
                                          infos.surfaceCapabilities.maxImageExtent.width);
    uint32_t height = std::clamp<uint32_t>(infos.surfaceCapabilities.currentExtent.height,
                                           infos.surfaceCapabilities.minImageExtent.height,
                                           infos.surfaceCapabilities.maxImageExtent.height);
    settings->setWindowSize(width, height);

    vk::SwapchainCreateInfoKHR info = {};
    info.setOldSwapchain(nullptr)
        .setClipped(true)
        .setSurface(manager->surface)
        .setPreTransform(infos.surfaceCapabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setImageFormat(format.format)
        .setImageColorSpace(format.colorSpace)
        .setImageArrayLayers(1)
        .setImageExtent({width, height})
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setMinImageCount(imageCount);
    
    uint32_t queueFamilyIndices[] = {manager->device->graphicsQueueFamilyIndex,
                                     manager->device->presentQueueFamilyIndex};
    if (queueFamilyIndices[0] == queueFamilyIndices[1]) {
        info.setImageSharingMode(vk::SharingMode::eExclusive);
    } else {
        info.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    }

    swapchain = manager->device->device.createSwapchainKHR(info);
    images = manager->device->device.getSwapchainImagesKHR(swapchain);
    imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        imageViews[i] = createImageView(images[i], format.format, vk::ImageAspectFlagBits::eColor, 1);
    }
}

Swapchain::~Swapchain() {
    for (auto& imageView : imageViews) {
        manager->device->device.destroyImageView(imageView);
    }
    imageViews.clear();
    images.clear();
    manager->device->device.destroySwapchainKHR(swapchain);
}

} // namespace wen