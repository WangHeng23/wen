#pragma once

#include "camera.hpp"
#include "application.hpp"
#include "renderer.hpp"

class RayTracing : public Layer {
public:
    RayTracing();
    ~RayTracing() override;

    void update(float ts) override;
    void render() override;

    void setCamera(const glm::vec3& position, const glm::vec3& direction);

private:
    Renderer renderer_;

    Camera camera_;
    Scene scene_;

    uint32_t width_ = 0;
    uint32_t height_ = 0;
};