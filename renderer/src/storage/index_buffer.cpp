
#include "storage/index_buffer.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

IndexBuffer::IndexBuffer(IndexType type, uint32_t count, vk::BufferUsageFlags additionalUsage) {
    indexType = convert<vk::IndexType>(type);
    staging_ = std::make_unique<Buffer>(
        static_cast<uint64_t>(count) * convert<uint32_t>(type),
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
    buffer_ = std::make_unique<Buffer>(
        static_cast<uint64_t>(count) * convert<uint32_t>(type),
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | additionalUsage,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
}

void* IndexBuffer::map() {
    return staging_->map();
}

void IndexBuffer::flush() {
    auto cmdbuf = manager->commandPool->allocateSingleUse();
    vk::BufferCopy regions = {};
    regions.setSize(staging_->size)
           .setSrcOffset(0)
           .setDstOffset(0);
    cmdbuf.copyBuffer(staging_->buffer, buffer_->buffer, regions);
    manager->commandPool->freeSingleUse(cmdbuf);
}

void IndexBuffer::unmap() {
    staging_->unmap();
}

IndexBuffer::~IndexBuffer() {
    buffer_.reset();
    staging_.reset();
}

} // namespace wen