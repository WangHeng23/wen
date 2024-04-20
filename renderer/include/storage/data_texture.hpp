#pragma once

#include "resources/texture.hpp"
#include "resources/image.hpp"

namespace wen {

class DataTexture : public Texture {
public:
    DataTexture(const uint8_t* data, uint32_t width, uint32_t height, uint32_t mipLevels);
    ~DataTexture() override;

    vk::ImageLayout getImageLayout() override { return vk::ImageLayout::eShaderReadOnlyOptimal; }
    vk::ImageView getImageView() override { return imageView_; }
    uint32_t getMipLevels() override { return mipLevels_; }

private:
    std::unique_ptr<Image> image_;
    vk::ImageView imageView_;
    uint32_t mipLevels_;
};

} // namespace wen