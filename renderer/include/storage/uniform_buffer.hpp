#pragma once

#include "resources/buffer.hpp"

namespace wen {

class UniformBuffer {
public:
    UniformBuffer(uint64_t size, bool inFlight);
    ~UniformBuffer();

    vk::Buffer getBuffer(uint32_t inFlight);
    uint64_t getSize() { return size_; }
    void* getData();

private:
    Buffer& buffer(uint32_t inFlight);

private:
    uint64_t size_;
    std::vector<Buffer> buffers_;
};

} // namespace wen