#pragma once

#include "resources/buffer.hpp"
#include "core/setting.hpp"

namespace wen {

class UniformBuffer : public StorageBuffer {
public:
    UniformBuffer(uint64_t size, bool inFlight);
    ~UniformBuffer() override;

    vk::Buffer getBuffer(uint32_t inFlight = 0) override { return buffer(inFlight).getBuffer(); }
    uint64_t getSize() override { return size_; }
    void* getData() override { return buffer(settings->maxFramesInFlight).getData(); };

private:
    Buffer& buffer(uint32_t inFlight);

private:
    uint64_t size_;
    std::vector<Buffer> buffers_;
};

} // namespace wen