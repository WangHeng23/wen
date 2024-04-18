#include "storage/vertex_buffer.hpp"
#include "manager.hpp"

namespace wen {

VertexBuffer::VertexBuffer(uint32_t size, uint32_t count, vk::BufferUsageFlags additionalUsage) {
    staging_ = std::make_unique<Buffer>(
        static_cast<uint64_t>(size) * count,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
    buffer_ = std::make_unique<Buffer>(
        static_cast<uint64_t>(size) * count,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst | additionalUsage,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
}

void* VertexBuffer::map() {
    return staging_->map();
}

void VertexBuffer::flush() {
    auto cmdbuf = manager->commandPool->allocateSingleUse();
    vk::BufferCopy regions = {};
    regions.setSize(staging_->size)
           .setSrcOffset(0)
           .setDstOffset(0);
    cmdbuf.copyBuffer(staging_->buffer, buffer_->buffer, regions);
    manager->commandPool->freeSingleUse(cmdbuf);
}

void VertexBuffer::unmap() {
    staging_->unmap();
}

VertexBuffer::~VertexBuffer() {
    buffer_.reset();
    staging_.reset();
}

} // namespace wen