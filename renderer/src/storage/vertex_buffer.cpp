#include "storage/vertex_buffer.hpp"
#include "manager.hpp"

namespace wen {

VertexBuffer::VertexBuffer(uint32_t size, uint32_t count, vk::BufferUsageFlags additionalUsage) {
    staging_ = std::make_unique<Buffer>(
        static_cast<uint64_t>(size) * count,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
    );
    buffer_ = std::make_unique<Buffer>(
        static_cast<uint64_t>(size) * count,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst | additionalUsage,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );
}

void* VertexBuffer::map() {
    return staging_->map();
}

void VertexBuffer::flush() {
    auto cmdbuf = manager->commandPool->allocateSingleUse();
    vk::BufferCopy regions = {};
    regions.setSize(staging_->getSize())
           .setSrcOffset(0)
           .setDstOffset(0);
    cmdbuf.copyBuffer(staging_->getBuffer(), buffer_->getBuffer(), regions);
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