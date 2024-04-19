#include "storage/descriptor_set.hpp"
#include "utils/utils.hpp"
#include "core/logger.hpp"
#include "core/setting.hpp"
#include "manager.hpp"

namespace wen {

DescriptorSet::DescriptorSet() {

}

DescriptorSet::~DescriptorSet() {
    freeDescriptorSets();
    destroyDescriptorSetLayout();
    layoutBindings_.clear();
}

DescriptorSet& DescriptorSet::addDescriptors(const std::vector<DescriptorInfo>& infos) {
    for (auto& info : infos) {
        layoutBindings_.push_back({
            info.binding,
            convert<vk::DescriptorType>(info.type),
            info.count,
            convert<vk::ShaderStageFlags>(info.stage),
            info.samplers
        });
    }
    return *this;
}

void DescriptorSet::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.setBindings(layoutBindings_);
    descriptorLayout_ = manager->device->device.createDescriptorSetLayout(createInfo);
}

void DescriptorSet::allocateDescriptorSets() {
    descriptorSets_ = manager->descriptorPool->allocateDescriptorSets(descriptorLayout_);
}

void DescriptorSet::freeDescriptorSets() {
    manager->descriptorPool->freeDescriptorSets(descriptorSets_);
    descriptorSets_.clear();
}

void DescriptorSet::destroyDescriptorSetLayout() {
    manager->device->device.destroyDescriptorSetLayout(descriptorLayout_);
}

void DescriptorSet::build() {
    createDescriptorSetLayout();
    allocateDescriptorSets();
}

const vk::DescriptorSetLayoutBinding& DescriptorSet::getLayoutBinding(uint32_t binding) {
    for (const auto& layoutBinding : layoutBindings_) {
        if (layoutBinding.binding == binding) {
            return layoutBinding;
        }
    }
    WEN_ERROR("DescriptorSetLayoutBinding \"{}\" not found!", binding);
    return *layoutBindings_.end();
}

void DescriptorSet::bindUniforms(uint32_t binding, const std::vector<std::shared_ptr<UniformBuffer>>& uniforms) {
    auto layoutBinding = getLayoutBinding(binding);
    if (layoutBinding.descriptorType != vk::DescriptorType::eUniformBuffer) {
        WEN_ERROR("binding {} is not uniform buffer!", binding);
        return;
    }
    assert(layoutBinding.descriptorCount == uniforms.size());
    for (uint32_t i = 0; i < settings->maxFramesInFlight; i++) {
        std::vector<vk::DescriptorBufferInfo> infos(layoutBinding.descriptorCount);
        for (uint32_t j = 0; j < layoutBinding.descriptorCount; j++) {
            infos[j].setBuffer(uniforms[j]->getBuffer(i))
                    .setOffset(0)
                    .setRange(uniforms[j]->getSize());
        }
        vk::WriteDescriptorSet write = {};
        write.setDstSet(descriptorSets_[i])
             .setDstBinding(layoutBinding.binding)
             .setDstArrayElement(0)
             .setDescriptorType(layoutBinding.descriptorType)
             .setBufferInfo(infos);
        manager->device->device.updateDescriptorSets({write}, {});
    }
}

void DescriptorSet::bindUniform(uint32_t binding, std::shared_ptr<UniformBuffer> uniform) {
    bindUniforms(binding, {uniform});
}

} // namespace wen