#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

class DescriptorPool {
public:
    DescriptorPool();
    ~DescriptorPool();

    std::vector<vk::DescriptorSet> allocateDescriptorSets(vk::DescriptorSetLayout descriptorLayout);
    void freeDescriptorSets(const std::vector<vk::DescriptorSet>& descriptorSets);

private:
    vk::DescriptorPool descriptorPool_;
};

} // namespace wen