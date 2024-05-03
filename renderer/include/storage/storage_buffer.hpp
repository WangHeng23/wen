#pragma once

#include "resources/buffer.hpp"
#include "resources/specific_buffer.hpp"

namespace wen {

class StorageBuffer : public SpecificBuffer {
public:
    StorageBuffer(uint64_t size, vk::BufferUsageFlags usage, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags);
    ~StorageBuffer() override;

    void* map();
    void flush(vk::DeviceSize size, const vk::Buffer& buffer);
    void unmap();

    vk::Buffer getBuffer(uint32_t inFlight = 0) override { return buffer_->buffer; }
    uint64_t getSize() override { return buffer_->size; }
    void* getData() override { return buffer_->data; }

private:
    std::unique_ptr<Buffer> buffer_;
};

} // namespace wen