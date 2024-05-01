#include "storage/storage_image.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

StorageImage::StorageImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags additionalUsage) {
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eStorage;
    image_ = std::make_unique<Image>(
        width, height,
        format,
        usage | additionalUsage,
        vk::SampleCountFlagBits::e1,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        1
    );
    transitionImageLayout(
        image_->image,
        vk::ImageAspectFlagBits::eColor, 1,
        {
            vk::ImageLayout::eUndefined,
            vk::AccessFlagBits::eNone,
            vk::PipelineStageFlagBits::eTopOfPipe
        },
        {
            vk::ImageLayout::eGeneral,
            vk::AccessFlagBits::eMemoryRead,
            vk::PipelineStageFlagBits::eAllGraphics
        }
    );
    imageView_ = createImageView(image_->image, format, vk::ImageAspectFlagBits::eColor, 1);
}

StorageImage::~StorageImage()  {
    manager->device->device.destroyImageView(imageView_);
    image_.reset();
};

} // namespace wen