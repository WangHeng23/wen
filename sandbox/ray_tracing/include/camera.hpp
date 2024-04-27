#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera(float fov, float near, float far);

    bool update(float ts);
    void resize(uint32_t width, uint32_t height);

    glm::mat4 view{1.0f};
    glm::mat4 projection{1.0f};

    glm::vec3 position;
    glm::vec3 direction;

private:
    uint32_t width_ = 0, height_ = 0;

    float fov_ = 60.0f;
    float near_ = 0.1f;
    float far_ = 100.0f;

    bool isCursorLocked_ = false;
};