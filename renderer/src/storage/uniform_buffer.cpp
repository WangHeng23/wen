#include "storage/uniform_buffer.hpp"
#include "core/setting.hpp"

namespace wen {

UniformBuffer::UniformBuffer(uint64_t size, bool inFlight) {
    size_ = size;
    for (uint32_t i = 0; i < (inFlight ? settings->maxFramesInFlight : 1); i++) {
        buffers_.emplace_back(
            size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
        buffers_.back().map();
    }
}

UniformBuffer::~UniformBuffer() {
    buffers_.clear();
}

vk::Buffer UniformBuffer::getBuffer(uint32_t inFlight) {
    return buffer(inFlight).buffer;
}

void* UniformBuffer::getData() {
    return buffer(settings->maxFramesInFlight).data;
}

Buffer& UniformBuffer::buffer(uint32_t inFlight) {
    return buffers_[inFlight % buffers_.size()];
}

} // namespace wen