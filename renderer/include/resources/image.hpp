#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class Image final {
public:
    Image(
        uint32_t width, uint32_t height,
        vk::Format format,
        vk::ImageUsageFlags usage,
        vk::SampleCountFlagBits samples,
        vk::MemoryPropertyFlags properties,
        uint32_t mipLevels = 1
    );
    ~Image();

    vk::Image image;
    vk::DeviceMemory memory;
};

} // namespace wen