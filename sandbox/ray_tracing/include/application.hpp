#pragma once

#include <wen.hpp>

class Layer {
public:
    virtual ~Layer() = default;

    virtual void update(float ts) {}
    virtual void render() {}
};

class Application {
public:
    Application();
    ~Application();

    template<typename T>
    void pushLayer() {
        layers_.emplace_back(std::make_shared<T>());
    }

    void init();
    void run();

    [[maybe_unused]] static vk::Instance getInstance();
    static vk::PhysicalDevice getPhysicalDevice();
    static vk::Device getDevice();

    static Application& get();
    auto getWindow() { return window_; }

    static vk::CommandBuffer allocateSingleUse();
    static void freeSingleUse(vk::CommandBuffer cmdbuf);

    static void submitResourceFree(std::function<void()>&& func);

private:
    GLFWwindow* window_;
    std::vector<std::shared_ptr<Layer>> layers_;
};