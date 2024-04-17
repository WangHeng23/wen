#include "command_pool.hpp"
#include "manager.hpp"

namespace wen {

CommandPool::CommandPool(vk::CommandPoolCreateFlags flags) {
    vk::CommandPoolCreateInfo createInfo = {};
    createInfo.setFlags(flags)
              .setQueueFamilyIndex(manager->device->graphicsQueueFamilyIndex);
    commandPool_ = manager->device->device.createCommandPool(createInfo);
}

CommandPool::~CommandPool() {
    manager->device->device.destroyCommandPool(commandPool_);
}

std::vector<vk::CommandBuffer> CommandPool::allocateCommandBuffers(uint32_t count) {
    vk::CommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.setCommandPool(commandPool_)
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(count);
    return std::move(manager->device->device.allocateCommandBuffers(allocateInfo));
}

} // namespace wen