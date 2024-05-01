#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace wen {

class Image final {
public:
    Image(
        uint32_t width, uint32_t height,
        vk::Format format,
        vk::ImageUsageFlags usage,
        vk::SampleCountFlagBits samples,
        VmaMemoryUsage vmaUsage,
        VmaAllocationCreateFlags vmaFlags,
        uint32_t mipLevels = 1
    );
    ~Image();

    vk::Image image;

private:
    VmaAllocation allocation_;
};

} // namespace wen