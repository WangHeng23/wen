#include "storage/uniform_buffer.hpp"

namespace wen {

UniformBuffer::UniformBuffer(uint64_t size, bool inFlight) {
    size_ = size;
    for (uint32_t i = 0; i < (inFlight ? settings->maxFramesInFlight : 1); i++) {
        buffers_.emplace_back(
            size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        );
        buffers_.back().map();
    }
}

UniformBuffer::~UniformBuffer() {
    buffers_.clear();
}

Buffer& UniformBuffer::buffer(uint32_t inFlight) {
    return buffers_[inFlight % buffers_.size()];
}

} // namespace wen