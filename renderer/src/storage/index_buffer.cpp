
#include "storage/index_buffer.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

IndexBuffer::IndexBuffer(IndexType type, uint32_t count, vk::BufferUsageFlags additionalUsage) {
    indexType = convert<vk::IndexType>(type);
    staging_ = std::make_unique<Buffer>(
        static_cast<uint64_t>(count) * convert<uint32_t>(type),
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
    );
    buffer_ = std::make_unique<Buffer>(
        static_cast<uint64_t>(count) * convert<uint32_t>(type),
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | additionalUsage,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    );
}

void* IndexBuffer::map() {
    return staging_->map();
}

void IndexBuffer::flush() {
    auto cmdbuf = manager->commandPool->allocateSingleUse();
    vk::BufferCopy regions = {};
    regions.setSize(staging_->getSize())
           .setSrcOffset(0)
           .setDstOffset(0);
    cmdbuf.copyBuffer(staging_->getBuffer(), buffer_->getBuffer(), regions);
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