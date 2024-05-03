#pragma once

#include "resources/buffer.hpp"
#include "resources/specific_buffer.hpp"
#include "core/setting.hpp"

namespace wen {

class UniformBuffer : public SpecificBuffer {
public:
    UniformBuffer(uint64_t size, bool inFlight);
    ~UniformBuffer() override;

    vk::Buffer getBuffer(uint32_t inFlight = 0) override { return buffer(inFlight).buffer; }
    uint64_t getSize() override { return size_; }
    void* getData() override { return buffer(settings->maxFramesInFlight).data; };

private:
    Buffer& buffer(uint32_t inFlight);

private:
    uint64_t size_;
    std::vector<Buffer> buffers_;
};

} // namespace wen