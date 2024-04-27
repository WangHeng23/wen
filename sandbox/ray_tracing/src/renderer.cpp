#include "renderer.hpp"
#include "tools/random.hpp"
#include "resources/material.hpp"
#include <numeric>
#include <execution>
#include <glm/glm.hpp>

static uint32_t convert(glm::vec4 color) {
    color = glm::sqrt(color);
    color = glm::clamp(color, glm::vec4(0.001f), glm::vec4(0.999f));
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
            for (int s = 0; s < samples; s++) {
                Ray ray = pixel(x, y, s);
                glm::vec4 color = glm::vec4(traceRay(ray, 50), 1.0f) / (float)samples;
                accumulation_[y * image_->width() + x] += color;
                color = accumulation_[y * image_->width() + x];
                color /= (float)index_;
                data_[y * image_->width() + x] = convert(color);
            } 
         });
    });
#else
    auto w = image_->width(), h = image_->height();
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            for (int s = 0; s < samples; s++) {
                Ray ray = pixel(x, y, s);
                glm::vec4 color = glm::vec4(traceRay(ray, 50), 1.0f) / (float)samples;
                accumulation_[y * w + x] += color;
                color = accumulation_[y * w + x];
                color /= (float)index_;
                data_[y * w + x] = convert(color);
            }
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

Ray Renderer::pixel(uint32_t x, uint32_t y, int s) {
    glm::vec2 coord = {
        (float)(x + s * (1.0f / (float)samples)) / (float)image_->width(),
        (float)(y + s * (1.0f / (float)samples)) / (float)image_->height()
    };
    coord = coord * 2.0f - 1.0f; // [0, 1] -> [-1, 1]
    glm::vec4 target = glm::inverse(camera_->projection) * glm::vec4(coord.x, coord.y, 1.0f, 1.0f);

    Ray ray {
        camera_->position,
        glm::vec3(glm::inverse(camera_->view) * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)),
        Random::Float()
    };

    return ray;
}

glm::vec3 Renderer::traceRay(const Ray& ray, int depth) {
    if (depth <= 0) {
        return glm::vec3(0.0f);
    }

    auto& world = scene_->world;
    HitRecord hitRecord;
    if (!world->hit(ray, Interval(0.001f, infinity), hitRecord)) {
        return background;
    }

    Ray rayOut;
    glm::vec3 attenuation;
    if (!hitRecord.material->scatter(ray, hitRecord, attenuation, rayOut)) {
        return glm::vec3(0.0f);
    }

    glm::vec3 color = traceRay(rayOut, depth - 1);

    return attenuation * color;
}