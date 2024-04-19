#include "descriptor_pool.hpp"
#include "core/setting.hpp"
#include "manager.hpp"

namespace wen {

DescriptorPool::DescriptorPool() {
    std::vector<vk::DescriptorPoolSize> sizes = {};
    vk::DescriptorPoolCreateInfo createInfo = {};
    createInfo.setPoolSizes(sizes)
              .setMaxSets(16)
              .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    descriptorPool_ = manager->device->device.createDescriptorPool(createInfo);
}

DescriptorPool::~DescriptorPool() {
    manager->device->device.destroyDescriptorPool(descriptorPool_);
}

std::vector<vk::DescriptorSet> DescriptorPool::allocateDescriptorSets(vk::DescriptorSetLayout descriptorLayout) {
    std::vector<vk::DescriptorSet> descriptorSets(settings->maxFramesInFlight);
    std::vector<vk::DescriptorSetLayout> layouts(settings->maxFramesInFlight, descriptorLayout);
    vk::DescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.setDescriptorPool(descriptorPool_)
                .setDescriptorSetCount(static_cast<uint32_t>(settings->maxFramesInFlight))
                .setSetLayouts(layouts);
    descriptorSets = manager->device->device.allocateDescriptorSets(allocateInfo);
    return descriptorSets;
}

void DescriptorPool::freeDescriptorSets(const std::vector<vk::DescriptorSet>& descriptorSets) {
    manager->device->device.freeDescriptorSets(descriptorPool_, descriptorSets);
}

}  // namespace wenEngine