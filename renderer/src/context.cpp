#include "manager.hpp"
#include "core/logger.hpp"
#include "core/setting.hpp"

namespace wen {

std::unique_ptr<Context> Context::instance_ = nullptr;

Context::Context() {
    ::wen::manager = this;
}

Context::~Context() {
    ::wen::manager = nullptr;
}

void Context::init() {
    if (instance_.get() == nullptr) {
        instance_.reset(new Context);
    }
}

Context& Context::instance() {
    return *instance_;
}

void Context::quit() {
    instance_.reset(nullptr);
}

void Context::initialize() {
    createVkInstance();
    if (settings->isEnableRayTracing) {
        vk::DynamicLoader dl;
        auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        dispatcher = vk::DispatchLoaderDynamic(vkInstance, vkGetInstanceProcAddr);
    }
    createSurface();
    device = std::make_unique<Device>();
    swapchain = std::make_unique<Swapchain>();
    commandPool = std::make_unique<CommandPool>(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    descriptorPool = std::make_unique<DescriptorPool>();
    initVMA();
    WEN_INFO("Vulkan Context Initialized!");
}

void Context::createVkInstance() {
    vk::ApplicationInfo app;
    app.setPApplicationName(settings->appName.c_str())
       .setApplicationVersion(settings->appVersion)
       .setPEngineName(settings->engineName.c_str())
       .setEngineVersion(settings->engineVersion)
       .setApiVersion(vk::ApiVersion13);
    vk::InstanceCreateInfo info;
    info.setPApplicationInfo(&app);

    std::map<std::string, bool> requiredExtensions;
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        requiredExtensions.insert(std::make_pair(std::string(glfwExtensions[i]), false));
    }

    std::vector<const char*> extensions;
    auto Ex = vk::enumerateInstanceExtensionProperties();
    bool found = false;
    for (const auto& ex : Ex) {
        if (requiredExtensions.find(ex.extensionName) != requiredExtensions.end()) {
            requiredExtensions[ex.extensionName] = true;
            extensions.push_back(ex.extensionName);
            found = true;
        }
    }
    for (const auto& [ex, success] : requiredExtensions) {
        WEN_ASSERT(success, "{} don't found in requiredExtensions", ex)
    }
    info.setPEnabledExtensionNames(extensions);

    const char* layers[] = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"};
    if (settings->debug) {
        auto La = vk::enumerateInstanceLayerProperties();
        bool found0 = false, found1 = false;
        for (const auto& la : La) {
            if (strcmp(la.layerName, layers[0]) == 0) {
                found0 = true;
            }
        }
        for (const auto& la : La) {
            if (strcmp(la.layerName, layers[1]) == 0) {
                found1 = true;
            }
        }
        if (found0 && found1) {
            info.setPEnabledLayerNames(layers);
        } else {
            WEN_ERROR("Validation layer or/and Monitor layer not support!")
        }
    }

    vkInstance = vk::createInstance(info);
}

void Context::createSurface() {
    VkSurfaceKHR surface;
    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->getWindow());
    WEN_ASSERT(
        glfwCreateWindowSurface(static_cast<VkInstance>(vkInstance), glfwWindow, nullptr, &surface) == VK_SUCCESS,
        "Failed to create window surface!"
    )
    this->surface = vk::SurfaceKHR(surface);
}

void Context::initVMA() {
    VmaAllocatorCreateInfo createInfo = {};
    if (settings->isEnableRayTracing) {
        createInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }
    createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    createInfo.instance = vkInstance;
    createInfo.physicalDevice = device->physicalDevice;
    createInfo.device = device->device;
    vmaCreateAllocator(&createInfo, &vmaAllocator);
}

void Context::recreateSwapchain() {
    int width = 0, height = 0;
    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->getWindow());
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(glfwWindow, &width, &height);
        glfwWaitEvents();
    }
    device->device.waitIdle();
    swapchain.reset();
    swapchain = std::make_unique<Swapchain>();
}

void Context::destroy() {
    vmaDestroyAllocator(vmaAllocator);
    descriptorPool.reset();
    commandPool.reset();
    swapchain.reset();
    device.reset();
    vkInstance.destroySurfaceKHR(surface);
    vkInstance.destroy();
    WEN_INFO("Vulkan Context Destroyed!");
}

} // namespace wen