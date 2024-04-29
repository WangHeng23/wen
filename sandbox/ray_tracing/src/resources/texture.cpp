#include "resources/textures.hpp"
#include "tools/interval.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

SolidColor::SolidColor(const glm::vec3& color) : color_(color) {}

SolidColor::SolidColor(float r, float g, float b) : color_(glm::vec3(r, g, b)) {}

glm::vec3 SolidColor::value(float u, float v, const glm::vec3& hitPoint) const {
    return color_;
}

// ChessboardTexture
ChessboardTexture::ChessboardTexture(float scale, const std::shared_ptr<Texture>& odd, const std::shared_ptr<Texture>& even)
    : scale_(scale), odd_(odd), even_(even) {}

ChessboardTexture::ChessboardTexture(float scale, const glm::vec3& odd, const glm::vec3& even)
    : scale_(scale),
      odd_(std::make_shared<SolidColor>(odd)),
      even_(std::make_shared<SolidColor>(even)) {}

glm::vec3 ChessboardTexture::value(float u, float v, const glm::vec3& hitPoint) const {
    int x = int(std::floor(1.0f / scale_ * hitPoint.x));
    int y = int(std::floor(1.0f / scale_ * hitPoint.y));
    int z = int(std::floor(1.0f / scale_ * hitPoint.z));

    bool even = (x + y + z) % 2 == 0;
    return even ? even_->value(u, v, hitPoint) : odd_->value(u, v, hitPoint);
}

// ImageTexture
ImageTexture::ImageTexture(const std::string& filename) {
    int channels;
    if (stbi_is_hdr(filename.c_str())) {
        data_ = (uint8_t*)stbi_loadf(filename.c_str(), &width_, &height_, &channels, STBI_rgb_alpha);
    } else {
        data_ = stbi_load(filename.c_str(), &width_, &height_, &channels, STBI_rgb_alpha);
    }
}

glm::vec3 ImageTexture::value(float u, float v, const glm::vec3& hitPoint) const {
    if (height_ <= 0) {
        return glm::vec3(0.0f, 1.0f, 1.0f);
    }

    u = Interval(0.0f, 1.0f).clamp(u);
    v = 1.0f - Interval(0.0f, 1.0f).clamp(v);

    auto i = static_cast<int>(u * width_);
    auto j = static_cast<int>(v * height_);
    auto pixel = pixels(i, j);

    float r = 1.0f / 255.0f;
    return glm::vec3(r * pixel[0], r * pixel[1], r * pixel[2]);
}

const uint8_t* ImageTexture::pixels(int x, int y) const {
    static uint8_t magenta[] = {255, 0, 255};
    if (data_ == nullptr) {
        return magenta;
    }

    x = std::clamp<int>(x, 0, width_ - 1);
    y = std::clamp<int>(y, 0, height_ - 1);

    return (uint8_t*)data_ + y * width_ * 4 + x * 4;
}

// NoiseTexture
NoiseTexture::NoiseTexture(float scale) : scale_(scale) {}

glm::vec3 NoiseTexture::value(float u, float v, const glm::vec3& hitPoint) const {
    auto s = scale_ * hitPoint;
    return glm::vec3(1.0f) * 0.5f * (1.0f + std::sin(s.z + 10.0f * noise_.turb(s, 7)));
}