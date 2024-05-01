#pragma once

#include "device.hpp"
#include "swapchain.hpp"
#include "interface.hpp"
#include "command_pool.hpp"
#include "descriptor_pool.hpp"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace wen {

class Context final {
public:
    ~Context();

    static void init();
    static Context& instance();
    static void quit();

    void initialize();
    void destroy();

    std::shared_ptr<Interface> createInterface(const std::string& path) {
        return std::make_shared<Interface>(path);
    }
    void recreateSwapchain();

public:
    vk::DispatchLoaderDynamic dispatcher;
    vk::Instance vkInstance;
    vk::SurfaceKHR surface;
    std::unique_ptr<Device> device;
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<CommandPool> commandPool;
    std::unique_ptr<DescriptorPool> descriptorPool;
    VmaAllocator vmaAllocator;

private:
    void createVkInstance();
    void createSurface();
    void initVMA();

private:
    Context();

private:
    static std::unique_ptr<Context> instance_;
};

} // namespace wen