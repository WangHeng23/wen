#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class Device final {
public:
    Device();
    ~Device();

public:
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

    vk::Queue graphicsQueue;
    uint32_t graphicsQueueFamilyIndex = -1;
    vk::Queue presentQueue;
    uint32_t presentQueueFamilyIndex = -1;
    vk::Queue transferQueue;
    uint32_t transferQueueFamilyIndex = -1;
    vk::Queue computeQueue;
    uint32_t computeQueueFamilyIndex = -1;

private:
    bool suitable(const vk::PhysicalDevice& device);
};

} // namespace wen