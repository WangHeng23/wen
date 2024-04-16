#pragma once

#include "device.hpp"
#include "swapchain.hpp"
#include <vulkan/vulkan.hpp>

namespace wen {

class Context final {
public:
    ~Context();

    static void init();
    static Context& instance();
    static void quit();

    void initialize();
    void destroy();

    void recreateSwapchain();

public:
    vk::Instance vkInstance;
    vk::SurfaceKHR surface;
    std::unique_ptr<Device> device;
    std::unique_ptr<Swapchain> swapchain;

private:
    void createVkInstance();
    void createSurface();

private:
    Context();

private:
    static std::unique_ptr<Context> instance_;
};

} // namespace wen