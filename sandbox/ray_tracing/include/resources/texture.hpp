#pragma once

#include "resources/perlin.hpp"

class Texture {
public:
    virtual ~Texture() = default;
    [[nodiscard]] virtual glm::vec3 value(float u, float v, const glm::vec3& hitPoint) const = 0;
};

class SolidColor : public Texture {
public:
    explicit SolidColor(const glm::vec3& color);
    SolidColor(float r, float g, float b);

    [[nodiscard]] glm::vec3 value(float u, float v, const glm::vec3& hitPoint) const override;

private:
    glm::vec3 color_;
};

class ChessboardTexture : public Texture {
public:
    ChessboardTexture(float scale, const std::shared_ptr<Texture>& odd, const std::shared_ptr<Texture>& even);
    ChessboardTexture(float scale, const glm::vec3& odd, const glm::vec3& even);

    [[nodiscard]] glm::vec3 value(float u, float v, const glm::vec3& hitPoint) const override;

private:
    float scale_;
    std::shared_ptr<Texture> odd_;
    std::shared_ptr<Texture> even_;
};

class ImageTexture : public Texture {
public:
    ImageTexture(const std::string& filename);

    [[nodiscard]] glm::vec3 value(float u, float v, const glm::vec3& hitPoint) const override;

private:
    const uint8_t* pixels(int x, int y) const;

private:
    int width_, height_;
    void* data_;
};

class NoiseTexture : public Texture {
public:
    NoiseTexture() = default;
    NoiseTexture(float scale);

    glm::vec3 value(float u, float v, const glm::vec3& hitPoint) const override;

private:
    Perlin noise_;
    float scale_;
};