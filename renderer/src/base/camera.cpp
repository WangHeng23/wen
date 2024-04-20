#include "base/camera.hpp"
#include "core/setting.hpp"
#include "base/window.hpp"
#include "base/key_codes.hpp"
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace wen {

Camera::Camera() {
    data.position = glm::vec3(0.0f, 0.0f, 3.0f);
    direction = glm::vec3(0.0f, 0.0f, -1.0f);
    uniform = std::make_shared<UniformBuffer>(sizeof(CameraData), false);
    upload();
}

void Camera::update(float ts) {
    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->getWindow());

    static glm::vec2 last = {0.0f, 0.0f};
    static bool first = true;
    double x, y;
    glfwGetCursorPos(glfwWindow, &x, &y);
    glm::vec2 now = {x, y};
    if (first) {
        first = false;
        last = now;
        return;
    }

    static bool spaceDown = false;
    int spaceState = glfwGetKey(glfwWindow, GLFW_KEY_SPACE);
    if (!spaceDown && spaceState == GLFW_PRESS) {
        spaceDown = true;
    } else if (spaceDown && spaceState == GLFW_RELEASE) {
        spaceDown = false;
        isCursorLocked_ = !isCursorLocked_;
        glfwSetInputMode(glfwWindow, GLFW_CURSOR, isCursorLocked_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    if (!isCursorLocked_) {
        last = now;
        return;
    }

    constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
    glm::vec3 rightDirection = glm::cross(direction, upDirection);
    float speed = 3.0f;
    if (glfwGetKey(glfwWindow, WEN_KEY_W)) {
        data.position += direction * speed * ts;
    } else if (glfwGetKey(glfwWindow, WEN_KEY_S)) {
        data.position -= direction * speed * ts;
    }
    if (glfwGetKey(glfwWindow, WEN_KEY_A)) {
        data.position -= rightDirection * speed * ts;
    } else if (glfwGetKey(glfwWindow, WEN_KEY_D)) {
        data.position += rightDirection * speed * ts;
    }
    if (glfwGetKey(glfwWindow, WEN_KEY_Q)) {
        data.position += upDirection * speed * ts;
    } else if (glfwGetKey(glfwWindow, WEN_KEY_E)) {
        data.position -= upDirection * speed * ts;
    }

    glm::vec2 delta = (now - last) * 0.002f;
    last = now;
    if (delta.x != 0.0f || delta.y != 0.0f) {
        float pitch = delta.y * 0.3f;
        float yaw = delta.x * 0.3f;
        glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitch, rightDirection), glm::angleAxis(-yaw, upDirection)));
        direction = glm::rotate(q, direction);
    }

    upload();
}

void Camera::upload() {
    data.view = glm::lookAt(data.position, data.position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    auto [width, height] = settings->windowSize;
    auto w = static_cast<float>(width), h = static_cast<float>(height);
    data.project = glm::perspective(glm::radians(60.0f), w / h, 0.1f, 100.0f);
    memcpy(uniform->getData(), &data, sizeof(CameraData));
}

} // namespace wen