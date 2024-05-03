#include "storage/storage_buffer.hpp"
#include "manager.hpp"

namespace wen {

StorageBuffer::StorageBuffer(uint64_t size, vk::BufferUsageFlags usage, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags) {
    buffer_ = std::make_unique<Buffer>(size, usage | vk::BufferUsageFlagBits::eStorageBuffer, vmaUsage, vmaFlags);
}

void* StorageBuffer::map() {
    return buffer_->map();
}

void StorageBuffer::flush(vk::DeviceSize size, const vk::Buffer& buffer) {
    auto cmdbuf = manager->commandPool->allocateSingleUse();
    vk::BufferCopy regions = {};
    regions.setSize(size)
           .setSrcOffset(0)
           .setDstOffset(0);
    cmdbuf.copyBuffer(buffer_->buffer, buffer, regions);
    manager->commandPool->freeSingleUse(cmdbuf);
}

void StorageBuffer::unmap() {
    buffer_->unmap();
}

StorageBuffer::~StorageBuffer() {
    buffer_.reset();
}

} // namespace wen