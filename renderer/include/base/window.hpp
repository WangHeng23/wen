#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wen {

class Window final {
public:
    struct Info {
        std::string title;
        uint32_t width, height;

        Info(const std::string& title = "wen", uint32_t width = 1600, uint32_t height = 900)
            : title(title), width(width), height(height) {}
    };

public:
    Window(const Info& info);
    ~Window();

    void init(const Info& info);
    void pollEvents() const;
    bool shouldClose() const;
    void destroy();

    void* getWindow() const { return window_; }

private:
    GLFWwindow* window_;
    struct Data {
        std::string title;
        uint32_t width, height;
    } data_;
};

extern std::unique_ptr<Window> window;

} // namespace wen