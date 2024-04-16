#include "base/window.hpp"
#include "core/logger.hpp"

namespace wen {

std::unique_ptr<Window> window = nullptr;

Window::Window(const Info& info) {
    init(info);
}

Window::~Window() {
    destroy();
}

void Window::init(const Info& info) {
    data_.title = info.title;
    data_.width = info.width;
    data_.height = info.height;
    WEN_INFO("Create window:({0}, {1}, {2})", info.title, info.width, info.height)

    glfwSetErrorCallback([](int error, const char* description) {
        WEN_ERROR("GLFW Error ({0}): {1}", error, description);
    });

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window_ = glfwCreateWindow(info.width, info.height, info.title.c_str(), nullptr, nullptr);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

void Window::destroy() {
    glfwDestroyWindow(window_);
    window_ = nullptr;
    glfwTerminate();
}

} // namespace wen