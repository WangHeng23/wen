#include "resources/buffer.hpp"
#include "manager.hpp"

namespace wen {

Buffer::Buffer(uint64_t size, vk::BufferUsageFlags usage, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags)
    : size(size), mapped_(false), data(nullptr) {
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = vmaUsage;
    allocInfo.flags = vmaFlags;

    vk::BufferCreateInfo createInfo = {};
    createInfo.size = size;
    createInfo.usage = usage;

    vmaCreateBuffer(
        manager->vmaAllocator,
        reinterpret_cast<VkBufferCreateInfo*>(&createInfo),
        &allocInfo,
        reinterpret_cast<VkBuffer*>(&buffer),
        &allocation_,
        nullptr
    );
}

void* Buffer::map() {
    if (mapped_) {
        return data;
    }
    mapped_ = true;
    vmaMapMemory(manager->vmaAllocator, allocation_, &data);
    return data;
}

void Buffer::unmap() {
    if (!mapped_) {
        return;
    }
    vmaUnmapMemory(manager->vmaAllocator, allocation_);
    data = nullptr;
    mapped_ = false;
}

Buffer::~Buffer() {
    unmap();
    vmaDestroyBuffer(manager->vmaAllocator, buffer, allocation_);
}

} // namespace wen