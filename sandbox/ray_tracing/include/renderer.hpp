#pragma once

#include "image.hpp"

class Renderer {
public:
    Renderer() = default;
    ~Renderer() = default;

    void resize(uint32_t width, uint32_t height);
    void render();

    std::shared_ptr<Image> getImage() const { return image_; }

private:
    std::shared_ptr<Image> image_ = nullptr;
    uint32_t* data_ = nullptr;

    std::vector<uint32_t> horizontal_;
    std::vector<uint32_t> vertical_;
};