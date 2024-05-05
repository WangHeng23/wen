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

void DescriptorSet::bindTextures(uint32_t binding, const std::vector<std::pair<std::shared_ptr<Texture>, std::shared_ptr<Sampler>>>& textures_samplers) {
    auto layoutBinding = getLayoutBinding(binding);
    if (layoutBinding.descriptorType != vk::DescriptorType::eCombinedImageSampler) {
        WEN_ERROR("binding {} is not combined image sampler!", binding);
        return;
    }
    assert(layoutBinding.descriptorCount == textures_samplers.size());
    for (uint32_t i = 0; i < settings->maxFramesInFlight; i++) {
        std::vector<vk::DescriptorImageInfo> infos(layoutBinding.descriptorCount);
        for (uint32_t j = 0; j < layoutBinding.descriptorCount; j++) {
            infos[j].setImageLayout(textures_samplers[j].first->getImageLayout())
                    .setImageView(textures_samplers[j].first->getImageView())
                    .setSampler(textures_samplers[j].second->sampler);
        }
        vk::WriteDescriptorSet write = {};
        write.setDstSet(descriptorSets_[i])
             .setDstBinding(layoutBinding.binding)
             .setDstArrayElement(0)
             .setDescriptorType(layoutBinding.descriptorType)
             .setImageInfo(infos);
        manager->device->device.updateDescriptorSets({write}, {});
    }
}

void DescriptorSet::bindTexture(uint32_t binding, std::shared_ptr<Texture> texture, std::shared_ptr<Sampler> sampler) {
    bindTextures(binding, {{texture, sampler}});
}

void DescriptorSet::bindInputAttachments(uint32_t binding, const std::shared_ptr<Renderer>& renderer, const std::vector<std::pair<std::string, std::shared_ptr<Sampler>>>& names_samplers) {
    auto layoutBinding = getLayoutBinding(binding);
    if ((layoutBinding.descriptorType != vk::DescriptorType::eInputAttachment) &&
        (layoutBinding.descriptorType != vk::DescriptorType::eCombinedImageSampler)) {
        WEN_ERROR("binding {} is not a inputAttachment descriptor!", binding);
        return;
    }
    assert(layoutBinding.descriptorCount == names_samplers.size());
    for (uint32_t i = 0; i < settings->maxFramesInFlight; i++) {
        std::vector<vk::DescriptorImageInfo> infos(layoutBinding.descriptorCount);
        for (uint32_t j = 0; j < layoutBinding.descriptorCount; j++) {
            auto index = renderer->renderPass->getAttachmentIndex(names_samplers[j].first, true);
            infos[j].setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setImageView(renderer->framebufferStore->attachments_[index].imageView)
                    .setSampler(names_samplers[j].second->sampler);
        }
        vk::WriteDescriptorSet write = {};
        write.setDstSet(descriptorSets_[i])
             .setDstBinding(layoutBinding.binding)
             .setDstArrayElement(0)
             .setDescriptorType(layoutBinding.descriptorType)
             .setImageInfo(infos);
        manager->device->device.updateDescriptorSets({write}, {});
    }
}

void DescriptorSet::bindInputAttachment(uint32_t binding, const std::shared_ptr<Renderer>& renderer, const std::string& name, std::shared_ptr<Sampler> sampler) {
    bindInputAttachments(binding, renderer, {{name, sampler}});
}

void DescriptorSet::bindStorageBuffers(uint32_t binding, const std::vector<std::shared_ptr<StorageBuffer>>& storageBuffers) {
    auto layoutBinding = getLayoutBinding(binding);
    if (layoutBinding.descriptorType != vk::DescriptorType::eStorageBuffer) {
        WEN_ERROR("binding {} is not stroage buffer descriptor!", binding);
    }
    assert(layoutBinding.descriptorCount == storageBuffers.size());
    for (uint32_t i = 0; i < settings->maxFramesInFlight; i++) {
        std::vector<vk::DescriptorBufferInfo> infos(layoutBinding.descriptorCount);
        for (uint32_t j = 0; j < layoutBinding.descriptorCount; j++) {
            infos[j].setBuffer(storageBuffers[j]->getBuffer(i))
                    .setOffset(0)
                    .setRange(storageBuffers[j]->getSize());
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

void DescriptorSet::bindStorageBuffer(uint32_t binding, std::shared_ptr<StorageBuffer> storageBuffer) {
    bindStorageBuffers(binding, {storageBuffer});
}

void DescriptorSet::bindStorageImages(uint32_t binding, const std::vector<std::shared_ptr<StorageImage>>& storageImages) {
    auto layoutBinding = getLayoutBinding(binding);
    if (layoutBinding.descriptorType != vk::DescriptorType::eStorageImage) {
        WEN_ERROR("binding {} is not stroage image descriptor!", binding);
    }
    assert(layoutBinding.descriptorCount == storageImages.size());
    for (uint32_t i = 0; i < settings->maxFramesInFlight; i++) {
        std::vector<vk::DescriptorImageInfo> infos(layoutBinding.descriptorCount);
        for (uint32_t j = 0; j < layoutBinding.descriptorCount; j++) {
            infos[j].setImageLayout(storageImages[j]->getImageLayout())
                    .setImageView(storageImages[j]->getImageView());
        }
        vk::WriteDescriptorSet write = {};
        write.setDstSet(descriptorSets_[i])
             .setDstBinding(layoutBinding.binding)
             .setDstArrayElement(0)
             .setDescriptorType(layoutBinding.descriptorType)
             .setImageInfo(infos);
        manager->device->device.updateDescriptorSets({write}, {});
    }
}

void DescriptorSet::bindStorageImage(uint32_t binding, std::shared_ptr<StorageImage> storageImage) {
    bindStorageImages(binding, {storageImage});
}

void DescriptorSet::bindAccelerationStructures(uint32_t binding, const std::vector<std::shared_ptr<RayTracingInstance>>& instances) {
    auto layoutBinding = getLayoutBinding(binding);
    if (layoutBinding.descriptorType != vk::DescriptorType::eAccelerationStructureKHR) {
        WEN_ERROR("binding {} is not acceleration structure descriptor!", binding);
    }
    assert(layoutBinding.descriptorCount == instances.size());
    for (uint32_t i = 0; i < settings->maxFramesInFlight; i++) {
        std::vector<vk::AccelerationStructureKHR> as;
        as.reserve(instances.size());
        for (auto& instance : instances) {
            as.push_back(instance->tlas);
        }
        vk::WriteDescriptorSetAccelerationStructureKHR writeAs = {};
        writeAs.setAccelerationStructureCount(as.size())
               .setAccelerationStructures(as);
        vk::WriteDescriptorSet write = {};
        write.setDstSet(descriptorSets_[i])
             .setDstBinding(layoutBinding.binding)
             .setDstArrayElement(0)
             .setDescriptorType(layoutBinding.descriptorType)
             .setDescriptorCount(writeAs.accelerationStructureCount)
             .setPNext(&writeAs);
        manager->device->device.updateDescriptorSets({write}, {});
    }
}

void DescriptorSet::bindAccelerationStructure(uint32_t binding, std::shared_ptr<RayTracingInstance> instance) {
    bindAccelerationStructures(binding, {instance});
}

} // namespace wen