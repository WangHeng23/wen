#include "utils/utils.hpp"
#include "core/logger.hpp"
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

std::vector<char> readFile(const std::string& name) {
    std::vector<char> buffer(0);
    std::ifstream file(name, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        WEN_ERROR("Open file \"{}\" failed", name);
        return buffer;
    }

    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0);
    buffer.resize(size);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

} // namespace wen