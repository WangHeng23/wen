
#pragma once

#include "resources/buffer.hpp"
#include "utils/enums.hpp"

namespace wen {

class IndexBuffer {
public:
    IndexBuffer(IndexType type, uint32_t count, vk::BufferUsageFlags additionalUsage);
    ~IndexBuffer();

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

    std::unique_ptr<Buffer>& getBuffer() { return buffer_; }

public:
    vk::IndexType indexType;

private:
    std::unique_ptr<Buffer> staging_;
    std::unique_ptr<Buffer> buffer_;
};

} // namespace wen