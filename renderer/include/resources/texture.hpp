#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class Texture {
public:
    Texture() = default;
    virtual ~Texture() = default;
    virtual vk::ImageLayout getImageLayout() = 0;
    virtual vk::ImageView getImageView() = 0;
    virtual uint32_t getMipLevels() = 0;
};

} // namespace wen