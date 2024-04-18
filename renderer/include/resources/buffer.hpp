#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class Buffer final {
public:
    Buffer(uint64_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    ~Buffer();

    void* map();
    void unmap();

public:
    uint64_t size;
    vk::Buffer buffer;

private:
    bool mapped_;
    void* data_;
    vk::DeviceMemory memory_;
};

} // namespace wen