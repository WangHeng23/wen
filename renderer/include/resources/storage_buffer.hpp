#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class StorageBuffer {
public:
    StorageBuffer() = default;
    virtual ~StorageBuffer() = default;
    virtual vk::Buffer getBuffer(uint32_t inFlight = 0) = 0;
    virtual uint64_t getSize() = 0;
    virtual void* getData() = 0;
};

} // namespace wen