#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect,
                              uint32_t levelCount, uint32_t layerCount, vk::ImageViewType viewType) {
    vk::ImageViewCreateInfo info = {};
    info.setImage(image)
        .setFormat(format)
        .setViewType(viewType)
        .setComponents({
            vk::ComponentSwizzle::eR,
            vk::ComponentSwizzle::eG,
            vk::ComponentSwizzle::eB,
            vk::ComponentSwizzle::eA,
        })
        .setSubresourceRange({
            aspect,
            0,
            levelCount,
            0,
            layerCount
        });

    return manager->device->device.createImageView(info);
}

} // namespace wen