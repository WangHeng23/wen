#pragma once

#include "resources/buffer.hpp"

namespace wen {

class VertexBuffer {
public:
    VertexBuffer(uint32_t size, uint32_t count, vk::BufferUsageFlags additionalUsage);
    ~VertexBuffer();

    void* map();
    void flush();
    void unmap();

    template <class Type>
    void upload(const std::vector<Type>& data) {
        auto* ptr = static_cast<uint8_t*>(staging_->map()); 
        memcpy(ptr, data.data(), data.size() * sizeof(Type));
        flush();
        staging_->unmap();
    }

    std::unique_ptr<Buffer>& getBuffer() { return buffer_; }

private:
    std::unique_ptr<Buffer> staging_;
    std::unique_ptr<Buffer> buffer_;
};

} // namespace wen