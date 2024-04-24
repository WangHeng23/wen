#pragma once

#include "storage/uniform_buffer.hpp"
#include <glm/glm.hpp>

namespace wen {

class Camera final {
public:
    struct CameraData {
        alignas(16) glm::vec3 position{0.0f, 0.0f, 0.0f};
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 project;
    } data;

    glm::vec3 direction;
    std::shared_ptr<wen::UniformBuffer> uniformBuffer;

public:
    Camera();
    void update(float ts);
    void upload();

private:
    bool isCursorLocked_ = false;
};

} // namespace wen