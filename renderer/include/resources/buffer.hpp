#pragma once

#include "resources/storage_buffer.hpp"
#include <vk_mem_alloc.h>

namespace wen {

class Buffer : public StorageBuffer {
public:
    Buffer(uint64_t size, vk::BufferUsageFlags usage, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags vmaFlags);
    ~Buffer() override;

    void* map();
    void unmap();

    vk::Buffer getBuffer(uint32_t inFlight = 0) override { return buffer_; }
    uint64_t getSize() override { return size_; }
    void* getData() override { return data_; }

private:
    uint64_t size_;
    vk::Buffer buffer_;
    VmaAllocation allocation_;
    bool mapped_;
    void* data_;
};

} // namespace wen