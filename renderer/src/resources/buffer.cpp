#include "resources/buffer.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

Buffer::Buffer(uint64_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    : size(size), mapped_(false), data(nullptr) {
    vk::BufferCreateInfo createInfo = {};
    createInfo.setSize(size)
              .setUsage(usage)
              .setSharingMode(vk::SharingMode::eExclusive);
    buffer = manager->device->device.createBuffer(createInfo);

    vk::MemoryRequirements requirements = manager->device->device.getBufferMemoryRequirements(buffer);
    uint32_t memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, properties);
    vk::MemoryAllocateInfo allocateInfo = {};
    allocateInfo.setAllocationSize(requirements.size)
                .setMemoryTypeIndex(memoryTypeIndex);
    memory_ = manager->device->device.allocateMemory(allocateInfo);

    //! offset
    manager->device->device.bindBufferMemory(buffer, memory_, 0);
}

void* Buffer::map() {
    if (mapped_) {
        return data;
    }
    mapped_ = true;
    //! offset
    data = manager->device->device.mapMemory(memory_, 0, size);
    return data;
}

void Buffer::unmap() {
    if (!mapped_) {
        return;
    }
    manager->device->device.unmapMemory(memory_);
    data = nullptr;
    mapped_ = false;
}

Buffer::~Buffer() {
    manager->device->device.freeMemory(memory_);
    manager->device->device.destroyBuffer(buffer);
}

} // namespace wen