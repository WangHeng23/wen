#include "resources/buffer.hpp"
#include "manager.hpp"

namespace wen {

Buffer::Buffer(uint64_t size, vk::BufferUsageFlags usage, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags)
    : size_(size), mapped_(false), data_(nullptr) {
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
        reinterpret_cast<VkBuffer*>(&buffer_),
        &allocation_,
        nullptr
    );
}

void* Buffer::map() {
    if (mapped_) {
        return data_;
    }
    mapped_ = true;
    vmaMapMemory(manager->vmaAllocator, allocation_, &data_);
    return data_;
}

void Buffer::unmap() {
    if (!mapped_) {
        return;
    }
    vmaUnmapMemory(manager->vmaAllocator, allocation_);
    data_ = nullptr;
    mapped_ = false;
}

Buffer::~Buffer() {
    unmap();
    vmaDestroyBuffer(manager->vmaAllocator, buffer_, allocation_);
}

} // namespace wen