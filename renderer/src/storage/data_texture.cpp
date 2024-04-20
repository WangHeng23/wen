#include "storage/data_texture.hpp"
#include "utils/utils.hpp"
#include "core/logger.hpp"
#include "manager.hpp"

namespace wen {

static void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
    vk::BufferImageCopy copy = {};
    copy.setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageSubresource({
            vk::ImageAspectFlagBits::eColor,
            0,
            0,
            1
        })
        .setImageOffset({0, 0, 0})
        .setImageExtent({width, height, 1});

    auto cmdbuf = manager->commandPool->allocateSingleUse();
    cmdbuf.copyBufferToImage(
        buffer,
        image,
        vk::ImageLayout::eTransferDstOptimal,
        {copy}
    );
    manager->commandPool->freeSingleUse(cmdbuf);
}

static void generateMipmaps(vk::Image image, vk::Format format, uint32_t width, uint32_t height, uint32_t mipLevels) {
    vk::FormatProperties formatProperties = manager->device->physicalDevice.getFormatProperties(format);
    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        WEN_ERROR("Texture image format does not support linear blitting");
        return;
    }

    auto cmdbuf = manager->commandPool->allocateSingleUse();
    vk::ImageMemoryBarrier barrier = {};
    barrier.setImage(image)
           .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
           .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
           .setSubresourceRange({
               vk::ImageAspectFlagBits::eColor,
               0,
               1,
               0,
               1
           });
    int32_t mipWidth = width, mipHeight = height;
    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.setBaseMipLevel(i - 1);
        barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
               .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
               .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
               .setDstAccessMask(vk::AccessFlagBits::eTransferRead);
        cmdbuf.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlagBits::eByRegion,
            {},
            {},
            {barrier}
        );

        vk::ImageBlit blit = {};
        blit.setSrcOffsets({{{0, 0, 0}, {mipWidth, mipHeight, 1}}})
            .setSrcSubresource({
                vk::ImageAspectFlagBits::eColor,
                i - 1,
                0,
                1
            })
            .setDstOffsets({{{0, 0, 0}, {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}}})
            .setDstSubresource({
                vk::ImageAspectFlagBits::eColor,
                i,
                0,
                1
            });
        cmdbuf.blitImage(
            image,
            vk::ImageLayout::eTransferSrcOptimal,
            image,
            vk::ImageLayout::eTransferDstOptimal,
            {blit},
            vk::Filter::eLinear
        );

        barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
               .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
               .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
               .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        cmdbuf.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlagBits::eByRegion,
            {},
            {},
            {barrier}
        );

        if (mipWidth > 1) {
            mipWidth /= 2;
        }
        if (mipHeight > 1) {
            mipHeight /= 2;
        }
    }
    barrier.subresourceRange.setBaseMipLevel(mipLevels - 1);
    barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
           .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
           .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
           .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
    cmdbuf.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::DependencyFlagBits::eByRegion,
        {},
        {},
        {barrier}
    );
    manager->commandPool->freeSingleUse(cmdbuf);
}

DataTexture::DataTexture(const uint8_t* data, uint32_t width, uint32_t height, uint32_t mipLevels) {
    uint32_t size = width * height * 4;
    Buffer stagingBuffer(
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
    memcpy(stagingBuffer.map(), data, size);
    stagingBuffer.unmap();

    if (mipLevels == 0) {
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
    }
    mipLevels_ = mipLevels;

    image_ = std::make_unique<Image>(
        width, height,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
        vk::SampleCountFlagBits::e1,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        mipLevels
    );

    transitionImageLayout(
        image_->image,
        vk::ImageAspectFlagBits::eColor,
        mipLevels,
        {
            vk::ImageLayout::eUndefined,
            vk::AccessFlagBits::eNone,
            vk::PipelineStageFlagBits::eTopOfPipe
        },
        {
            vk::ImageLayout::eTransferDstOptimal,
            vk::AccessFlagBits::eTransferWrite,
            vk::PipelineStageFlagBits::eTransfer
        }
    );

    copyBufferToImage(stagingBuffer.buffer, image_->image, width, height);
    generateMipmaps(image_->image, vk::Format::eR8G8B8A8Srgb, width, height, mipLevels);

    imageView_ = createImageView(image_->image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
}

DataTexture::~DataTexture() {
    manager->device->device.destroyImageView(imageView_);
    image_.reset();
}

} // namespace wen