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

vk::CommandBuffer CommandPool::allocateSingleUse() {
    vk::CommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.setCommandPool(commandPool_)
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(1);

    auto cmdbuf = manager->device->device.allocateCommandBuffers(allocateInfo)[0];
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdbuf.begin(beginInfo);

    return std::move(cmdbuf);
}

void CommandPool::freeSingleUse(vk::CommandBuffer cmdbuf) {
    cmdbuf.end();

    vk::SubmitInfo submits = {};
    submits.setCommandBuffers(cmdbuf);
    manager->device->transferQueue.submit(submits, nullptr);
    manager->device->transferQueue.waitIdle();
    manager->device->device.freeCommandBuffers(commandPool_, cmdbuf);
}

} // namespace wen