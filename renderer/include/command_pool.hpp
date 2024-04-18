#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class CommandPool final {
public:
    CommandPool(vk::CommandPoolCreateFlags flags);
    ~CommandPool();

    std::vector<vk::CommandBuffer> allocateCommandBuffers(uint32_t count);

    vk::CommandBuffer allocateSingleUse();
    void freeSingleUse(vk::CommandBuffer cmdbuf);

private:
    vk::CommandPool commandPool_;
};

} // namespace wen