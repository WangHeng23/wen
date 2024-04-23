#include "renderer.hpp"
#include "tools/random.hpp"
#include <numeric>
#include <execution>
#include <glm/glm.hpp>

static uint32_t convert(glm::vec4 color) {
    color = glm::pow(color, glm::vec4(1.0f / 2.2f));
    color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(0.999f));
    uint8_t r = static_cast<uint8_t>(color.r * 255.0f);
    uint8_t g = static_cast<uint8_t>(color.g * 255.0f);
    uint8_t b = static_cast<uint8_t>(color.b * 255.0f);
    uint8_t a = static_cast<uint8_t>(color.a * 255.0f);
    uint32_t result = a << 24 | b << 16 | g << 8 | r;
    return result;
}

void Renderer::resize(uint32_t width, uint32_t height) {
    if (image_ && image_->width() == width && image_->height() == height) {
        return;
    }

    if (image_) {
        image_->resize(width, height);
    } else {
        image_ = std::make_shared<Image>(width, height, ImageFormat::RGBA);
    }

    delete[] data_;
    data_ = new uint32_t[width * height];

    horizontal_.resize(width);
    vertical_.resize(height);
    std::iota(std::begin(horizontal_), std::end(horizontal_), 0);
    std::iota(std::begin(vertical_), std::end(vertical_), 0);
}

void Renderer::render() {

#define MT 1
#if MT
    std::for_each(std::execution::par, vertical_.begin(), vertical_.end(), [&](uint32_t y) {
        std::for_each(std::execution::par, horizontal_.begin(), horizontal_.end(), [&](uint32_t x) {
            data_[y * image_->width() + x] = convert(glm::vec4(Random::Vec3(), 1.0f));
        });
    });
#else
    auto w = image_->getWidth(), h = image_->getHeight();
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            data_[y * w + x] = convert(glm::vec4(Random::Vec3(), 1.0f));
        }
    }
#endif

    image_->set(data_);
}