#pragma once

#include "resources/texture.hpp"
#include <ktxvulkan.h>

namespace wen {

class KtxTexture : public Texture {
public:
    KtxTexture(const std::string& filename);
    ~KtxTexture() override;

    vk::ImageLayout getImageLayout() override { return vk::ImageLayout::eShaderReadOnlyOptimal; }
    vk::ImageView getImageView() override { return imageView_; }
    uint32_t getMipLevels() override { return vkTexture_.levelCount; }

private:
    ktxTexture* texture_;
    ktxVulkanTexture vkTexture_;
    vk::ImageView imageView_;
};

} // namespace wen