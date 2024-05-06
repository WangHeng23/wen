#include "storage/ktx_texture.hpp"
#include "core/logger.hpp"
#include "utils/utils.hpp"
#include "manager.hpp"

namespace wen {

KtxTexture::KtxTexture(const std::string& filename) {
    auto res = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &texture_);
    if (res != KTX_SUCCESS) {
        WEN_ERROR("Failed to load ktx texture image!");
        return;
    }

    ktxVulkanDeviceInfo deviceInfo = {};
    ktxVulkanDeviceInfo_Construct(
        &deviceInfo,
        manager->device->physicalDevice,
        manager->device->device,
        manager->device->transferQueue,
        manager->commandPool->commandPool_,
        nullptr
    );
    res = ktxTexture_VkUploadEx(
        texture_, &deviceInfo,
        &vkTexture_,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    if (res != KTX_SUCCESS) {
        WEN_ERROR("Failed to upload ktx texture image!");
        return;
    }

    imageView_ = createImageView(vkTexture_.image, vk::Format(vkTexture_.imageFormat), vk::ImageAspectFlagBits::eColor, vkTexture_.levelCount);

    ktxVulkanDeviceInfo_Destruct(&deviceInfo);
}

KtxTexture::~KtxTexture() {
    vkTexture_.vkFreeMemory(manager->device->device, vkTexture_.deviceMemory, nullptr);
    vkTexture_.vkDestroyImage(manager->device->device, vkTexture_.image, nullptr);
    ktxTexture_Destroy(texture_);
    manager->device->device.destroyImageView(imageView_);
}

} // namespace wen