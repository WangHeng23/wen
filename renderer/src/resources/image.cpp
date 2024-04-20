#include "resources/image.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

Image::Image(
    uint32_t width, uint32_t height,
    vk::Format format,
    vk::ImageUsageFlags usage,
    vk::SampleCountFlagBits samples,
    vk::MemoryPropertyFlags properties,
    uint32_t mipLevels
) {
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
    image = manager->device->device.createImage(createInfo);

    vk::MemoryRequirements requirements = manager->device->device.getImageMemoryRequirements(image);
    uint32_t memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, properties);
    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.setAllocationSize(requirements.size)
             .setMemoryTypeIndex(memoryTypeIndex);
    memory = manager->device->device.allocateMemory(allocInfo);

    manager->device->device.bindImageMemory(image, memory, 0);
}

Image::~Image() {
    manager->device->device.freeMemory(memory);
    manager->device->device.destroyImage(image);
}

} // namespace wen