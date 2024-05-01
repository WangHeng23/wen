#pragma once

#include "resources/image.hpp"
#include "resources/texture.hpp"

namespace wen {

class StorageImage : public Texture {
public:
    StorageImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags additionalUsage);
    ~StorageImage() override;

    vk::ImageLayout getImageLayout() override { return vk::ImageLayout::eGeneral; }
    vk::ImageView getImageView() override { return imageView_; }
    uint32_t getMipLevels() override { return 1; }

private:
    std::unique_ptr<Image> image_;
    vk::ImageView imageView_;
};

} // namespace wen