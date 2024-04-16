#include "device.hpp"
#include "core/setting.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

namespace wen {

Device::Device() {
    auto devices = manager->vkInstance.enumeratePhysicalDevices();
    bool selected = false;
    for (auto device : devices) {
        if (suitable(device)) {
            selected = true;
            break;
        }
    }
    if (!selected) {
        WEN_ERROR("No suitable physical device found!");
    } else {
        auto properties = physicalDevice.getProperties();
        WEN_INFO("Device selected: {}", properties.deviceName)
        if (settings->debug) {
            WEN_DEBUG(
                "  Device Driver Version: {}.{}.{}",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion)
            )
            WEN_DEBUG(
                "  Device API Version: {}.{}.{}",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion)
            )
            auto memoryProperties = physicalDevice.getMemoryProperties();
            for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
                float size = ((float)memoryProperties.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f;
                if (memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                    WEN_DEBUG("  Local GPU Memory: {} Gib", size)
                } else {
                    WEN_DEBUG("  Shared GPU Memory: {} Gib", size)
                }
            }
        }
    }

    vk::DeviceCreateInfo deviceCreateInfo;

    vk::PhysicalDeviceFeatures features = {};
    features.setSamplerAnisotropy(true)
            .setDepthClamp(true)
            .setSampleRateShading(true)
            .setShaderInt64(true)
            .setFragmentStoresAndAtomics(true)
            .setMultiDrawIndirect(true)
            .setGeometryShader(true)
            .setFillModeNonSolid(true)
            .setWideLines(true);
    deviceCreateInfo.setPEnabledFeatures(&features);
    vk::PhysicalDeviceVulkan12Features vk12Features = {};
    vk12Features.setRuntimeDescriptorArray(true)
                .setBufferDeviceAddress(true)
                .setShaderSampledImageArrayNonUniformIndexing(true)
                .setDescriptorBindingVariableDescriptorCount(true)
                .setDrawIndirectCount(true)
                .setSamplerFilterMinmax(true);
    deviceCreateInfo.setPNext(&vk12Features);

    // extensions
    std::map<std::string, bool> requiredExtensions = {
        {VK_KHR_SWAPCHAIN_EXTENSION_NAME, false},
    };
    for (const auto& ex : settings->deviceRequestedExtensions) {
        requiredExtensions.insert(std::make_pair(ex, false));
    }

    std::vector<const char*> extensions;
    auto Ex = physicalDevice.enumerateDeviceExtensionProperties();
    bool found = false;
    for (const auto& ex : Ex) {
        for (auto& requiredEx : requiredExtensions) {
            if (requiredEx.first == ex.extensionName) {
                requiredEx.second = true;
                extensions.push_back(ex.extensionName);
                break;
            }
        }
    }
    for (const auto& [ex, success] : requiredExtensions) {
        WEN_ASSERT(success, "Required extension not found: {}", ex);
    }
    deviceCreateInfo.setPEnabledExtensionNames(extensions);

    // layers
    if (settings->debug) {
        const char* layers[] = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"};
        deviceCreateInfo.setPEnabledLayerNames(layers);
    }

    // queue
    std::set<uint32_t> queueFamilyIndices = {graphicsQueueFamilyIndex, presentQueueFamilyIndex,
                                             transferQueueFamilyIndex, computeQueueFamilyIndex};
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos(queueFamilyIndices.size());
    uint32_t i = 0;
    float priorities = 1.0f;
    for (auto index : queueFamilyIndices) {
        queueCreateInfos[i].setQueueFamilyIndex(index)
                           .setQueueCount(1)
                           .setQueuePriorities(priorities);
        i++;
    }
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);

    device = physicalDevice.createDevice(deviceCreateInfo);

    graphicsQueue = device.getQueue(graphicsQueueFamilyIndex, 0);
    presentQueue = device.getQueue(presentQueueFamilyIndex, 0);
    transferQueue = device.getQueue(transferQueueFamilyIndex, 0);
    computeQueue = device.getQueue(computeQueueFamilyIndex, 0);
}

Device::~Device() {
    device.destroy();
}

bool Device::suitable(const vk::PhysicalDevice& device) {
    auto properties = device.getProperties();
    auto features = device.getFeatures();

    if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
        WEN_WARN("Device not discrete GPU: {}", properties.deviceName)
        return false;
    }

    if (!features.samplerAnisotropy) {
        WEN_WARN("Device does not support sampler anisotropy: {}", properties.deviceName)
        return false;
    }

    if (!features.depthClamp) {
        WEN_WARN("Device does not support depth clamp: {}", properties.deviceName)
        return false;
    }

    if (!features.sampleRateShading) {
        WEN_WARN("Device does not support sample rate shading: {}", properties.deviceName)
        return false;
    }

    if (!features.shaderInt64) {
        WEN_WARN("Device does not support shader int64: {}", properties.deviceName)
        return false;
    }

    auto queueFamilyProperties = device.getQueueFamilyProperties();
    uint32_t queueFamilyIndex = 0;
    bool found = false;
    for (const auto& queueFamily : queueFamilyProperties) {
        if ((queueFamily.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eCompute))
            && (device.getSurfaceSupportKHR(queueFamilyIndex, manager->surface))) {
            graphicsQueueFamilyIndex = queueFamilyIndex;
            presentQueueFamilyIndex = queueFamilyIndex;
            computeQueueFamilyIndex = queueFamilyIndex;
            transferQueueFamilyIndex = queueFamilyIndex;
            found = true;
            break;
        }
        queueFamilyIndex++;
    }
    if (!found) {
        WEN_WARN("Device does not support required queue family: {}", properties.deviceName)
        return false;
    }

    physicalDevice = device;

    return true;
}

} // namespace wen