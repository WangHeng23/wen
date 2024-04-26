#include "renderer.hpp"
#include "tools/random.hpp"
#include "resources/material.hpp"
#include <numeric>
#include <execution>
#include <glm/glm.hpp>

static uint32_t convert(glm::vec4 color) {
    color = glm::sqrt(color);
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

    delete[] accumulation_;
    accumulation_ = new glm::vec4[width * height];

    horizontal_.resize(width);
    vertical_.resize(height);
    std::iota(std::begin(horizontal_), std::end(horizontal_), 0);
    std::iota(std::begin(vertical_), std::end(vertical_), 0);
}

void Renderer::render(const Camera& camera, const Scene& scene) {
    camera_ = &camera;
    scene_ = &scene;

    if (index_ == 1) {
        memset(accumulation_, 0, image_->width() * image_->height() * sizeof(glm::vec4));
    }

#define MT 1
#if MT
    std::for_each(std::execution::par, vertical_.begin(), vertical_.end(), [&](uint32_t y) {
        std::for_each(std::execution::par, horizontal_.begin(), horizontal_.end(), [&](uint32_t x) {
            Ray ray = pixel(x, y);
            glm::vec4 color = glm::vec4(traceRay(ray, 10), 1.0f);
            accumulation_[y * image_->width() + x] += color;
            color = accumulation_[y * image_->width() + x];
            color /= (float)index_;
            data_[y * image_->width() + x] = convert(color);
        });
    });
#else
    auto w = image_->width(), h = image_->height();
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            Ray ray = pixel(x, y);
            glm::vec4 color = glm::vec4(traceRay(ray, 50), 1.0f);
            accumulation_[y * w + x] += color;
            color = accumulation_[y * w + x];
            color /= (float)index_;
            data_[y * w + x] = convert(color);
        }
    }
#endif

    image_->set(data_);

    if (accumulated_) {
        index_++;
    } else {
        index_ = 1;
    }
}

Ray Renderer::pixel(uint32_t x, uint32_t y) {
    Ray ray;
    ray.origin = camera_->position;
    ray.direction = camera_->rays[y * image_->width() + x];
    return ray;
}

glm::vec3 Renderer::traceRay(const Ray& ray, int depth) {
    if (depth <= 0) {
        return glm::vec3(0.0f);
    }

    auto& world = scene_->world;
    HitRecord hitRecord;
    if (world->hit(ray, Interval(0.001f, infinity), hitRecord)) {
        Ray rayOut;
        glm::vec3 attenuation;
        if (hitRecord.material->scatter(ray, hitRecord, attenuation, rayOut)) {
            return attenuation * traceRay(rayOut, depth - 1);
        } else {
            return glm::vec3(0.0f);
        }
    }

    float a = 0.5 * (glm::normalize(ray.direction).y + 1.0);
    return (1.0f - a) * glm::vec3(1.0f) + a * glm::vec3(0.5f, 0.7f, 1.0f);
}