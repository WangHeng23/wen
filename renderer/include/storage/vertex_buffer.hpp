#pragma once

#include "resources/buffer.hpp"

namespace wen {

class VertexBuffer : public StorageBuffer {
public:
    VertexBuffer(uint32_t size, uint32_t count, vk::BufferUsageFlags additionalUsage);
    ~VertexBuffer() override;

    void* map();
    void flush();
    void unmap();

    template <class Type>
    uint32_t upload(const std::vector<Type>& data, uint32_t offset = 0) {
        auto* ptr = static_cast<uint8_t*>(staging_->map()); 
        memcpy(ptr + (offset * sizeof(Type)), data.data(), data.size() * sizeof(Type));
        flush();
        staging_->unmap();
        return offset + data.size();
    }

    vk::Buffer getBuffer(uint32_t inFlight = 0) override { return buffer_->getBuffer(); }
    uint64_t getSize() override { return buffer_->getSize(); }
    void* getData() override { return buffer_->getData(); }

private:
    std::unique_ptr<Buffer> staging_;
    std::unique_ptr<Buffer> buffer_;
};

} // namespace wen