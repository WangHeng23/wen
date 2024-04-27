#include "camera.hpp"
#include "application.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

Camera::Camera(float fov, float near, float far) : fov_(fov), near_(near), far_(far) {}

bool Camera::update(float ts) {
    auto window = Application::get().getWindow();

    static glm::vec2 last = {0.0f, 0.0f};
    static bool first = true;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    glm::vec2 now = {x, y};
    if (first) {
        first = false;
        last = now;
        return false;
    }

    static bool spaceDown = false;
    int spaceState = glfwGetKey(window, GLFW_KEY_SPACE);
    if (!spaceDown && spaceState == GLFW_PRESS) {
        spaceDown = true;
    } else if (spaceDown && spaceState == GLFW_RELEASE) {
        spaceDown = false;
        isCursorLocked_ = !isCursorLocked_;
        glfwSetInputMode(window, GLFW_CURSOR, isCursorLocked_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    if (!isCursorLocked_) {
        last = now;
        return false;
    }

    bool moved = false;

    constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
    glm::vec3 rightDirection = glm::cross(direction, upDirection);
    float speed = 3.0f;
    if (glfwGetKey(window, GLFW_KEY_W)) {
        position += direction * speed * ts;
        moved = true;
    } else if (glfwGetKey(window, GLFW_KEY_S)) {
        position -= direction * speed * ts;
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        position -= rightDirection * speed * ts;
        moved = true;
    } else if (glfwGetKey(window, GLFW_KEY_D)) {
        position += rightDirection * speed * ts;
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Q)) {
        position += upDirection * speed * ts;
        moved = true;
    } else if (glfwGetKey(window, GLFW_KEY_E)) {
        position -= upDirection * speed * ts;
        moved = true;
    }

    glm::vec2 delta = (now - last) * 0.002f;
    last = now;
    if (delta.x != 0.0f || delta.y != 0.0f) {
        float pitch = delta.y * 0.3f;
        float yaw = delta.x * 0.3f;
        glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitch, rightDirection), glm::angleAxis(-yaw, upDirection)));
        direction = glm::rotate(q, direction);
        moved = true;
    }

    if (moved) {
        view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    return moved;
}

void Camera::resize(uint32_t width, uint32_t height) {
    if (width_ == width && height_ == height) {
        return;
    }

    width_ = width;
    height_ = height;

    view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspectiveFov(glm::radians(fov_), (float)width_, (float)height_, near_, far_);
}