
#pragma once

#include "resources/buffer.hpp"
#include "resources/specific_buffer.hpp"
#include "utils/enums.hpp"

namespace wen {

class IndexBuffer : public SpecificBuffer {
public:
    IndexBuffer(IndexType type, uint32_t count, vk::BufferUsageFlags additionalUsage);
    ~IndexBuffer() override;

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

    vk::Buffer getBuffer(uint32_t inFlight = 0) override { return buffer_->buffer; }
    uint64_t getSize() override { return buffer_->size; }
    void* getData() override { return buffer_->data; }

public:
    vk::IndexType indexType;

private:
    std::unique_ptr<Buffer> staging_;
    std::unique_ptr<Buffer> buffer_;
};

} // namespace wen