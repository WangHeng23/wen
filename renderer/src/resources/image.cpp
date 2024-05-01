#include "resources/image.hpp"
#include "manager.hpp"

namespace wen {

Image::Image(
    uint32_t width, uint32_t height,
    vk::Format format,
    vk::ImageUsageFlags usage,
    vk::SampleCountFlagBits samples,
    VmaMemoryUsage vmaUsage,
    VmaAllocationCreateFlags vmaFlags,
    uint32_t mipLevels
) {
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = vmaUsage;
    allocInfo.flags = vmaFlags;

    vk::ImageCreateInfo createInfo = {};
    createInfo.setImageType(vk::ImageType::e2D)
              .setExtent({width, height, 1})
              .setMipLevels(mipLevels)
              .setArrayLayers(1)
              .setFormat(format)
              .setTiling(vk::ImageTiling::eOptimal)
              .setUsage(usage)
              .setSamples(samples)
              .setInitialLayout(vk::ImageLayout::eUndefined);

    vmaCreateImage(
        manager->vmaAllocator,
        reinterpret_cast<VkImageCreateInfo*>(&createInfo),
        &allocInfo,
        reinterpret_cast<VkImage*>(&image),
        &allocation_,
        nullptr
    );
}

Image::~Image() {
    vmaDestroyImage(manager->vmaAllocator, image, allocation_);
}

} // namespace wen