#include "resources/sampler.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

Sampler::Sampler(const SamplerInfos& infos) {
    vk::SamplerCreateInfo createInfo = {};
    createInfo.setMagFilter(convert<vk::Filter>(infos.magFilter))
              .setMinFilter(convert<vk::Filter>(infos.minFilter))
              .setAddressModeU(convert<vk::SamplerAddressMode>(infos.addressModeU))
              .setAddressModeV(convert<vk::SamplerAddressMode>(infos.addressModeV))
              .setAddressModeW(convert<vk::SamplerAddressMode>(infos.addressModeW))
              .setAnisotropyEnable(VK_TRUE)
              .setMaxAnisotropy(infos.maxAnisotropy)
              .setBorderColor(convert<vk::BorderColor>(infos.borderColor))
              .setUnnormalizedCoordinates(VK_FALSE)
              .setCompareEnable(VK_FALSE)
              .setCompareOp(vk::CompareOp::eAlways)
              .setMipmapMode(convert<vk::SamplerMipmapMode>(infos.mipmapMode))
              .setMinLod(0.0f)
              .setMaxLod(static_cast<float>(infos.mipLevels))
              .setMipLodBias(0.0f);
    
    sampler = manager->device->device.createSampler(createInfo);
}

Sampler::~Sampler() {
    manager->device->device.destroySampler(sampler);
}

} // namespace wen