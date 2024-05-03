#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class SpecificBuffer {
public:
    SpecificBuffer() = default;
    virtual ~SpecificBuffer() = default;
    virtual vk::Buffer getBuffer(uint32_t inFlight = 0) = 0;
    virtual uint64_t getSize() = 0;
    virtual void* getData() = 0;
};

} // namespace wen