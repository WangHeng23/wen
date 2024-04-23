#include "image.hpp"
#include "application.hpp"

#include "backends/imgui_impl_vulkan.h"

namespace Utils {

static uint32_t getMemoryType(vk::MemoryPropertyFlags properties, uint32_t bits) {
    vk::PhysicalDeviceMemoryProperties prop;
    Application::getPhysicalDevice().getMemoryProperties(&prop);

    for (uint32_t i = 0; i < prop.memoryTypeCount; i++) {
        if ((prop.memoryTypes[i].propertyFlags & properties) == properties && bits & (1 << i)) {
            return i;
        }
    }

    return 0xffffffff;
}

static uint32_t BytesPerPixel(ImageFormat format) {
    switch (format) {
        case ImageFormat::RGBA:
            return 4;
        case ImageFormat::RGBA32F:
            return 16;
        default:
            return 0;
    }
}

static vk::Format convert(ImageFormat format) {
    switch (format) {
        case ImageFormat::RGBA:
            return vk::Format::eR8G8B8A8Unorm;
        case ImageFormat::RGBA32F:
            return vk::Format::eR32G32B32A32Sfloat;
        default:
            break;
    }
    return (vk::Format)0;
}

} // namespace Utils

Image::Image(uint32_t width, uint32_t height, ImageFormat format)
    : width_(width), height_(height), format_(format) {
    create(width_ * height_ * Utils::BytesPerPixel(format_));
}

Image::~Image() {
    reset();
}

void Image::create(uint64_t size) {
    auto device = Application::getDevice();
    vk::Format format = Utils::convert(format_);

    // create image
    {
        vk::ImageCreateInfo createInfo = {};
        createInfo.setImageType(vk::ImageType::e2D)
            .setFormat(format)
            .setExtent({width_, height_, 1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined);
        image_ = device.createImage(createInfo);

        vk::MemoryRequirements req = device.getImageMemoryRequirements(image_);
        vk::MemoryAllocateInfo allocateInfo = {};
        allocateInfo.setAllocationSize(req.size)
             .setMemoryTypeIndex(Utils::getMemoryType(vk::MemoryPropertyFlagBits::eDeviceLocal, req.memoryTypeBits));
        imageMemory_ = device.allocateMemory(allocateInfo);
        device.bindImageMemory(image_, imageMemory_, 0);
    }
    // create image view
    {
        vk::ImageViewCreateInfo createInfo = {};
        createInfo.setImage(image_)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(format)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        imageView_ = device.createImageView(createInfo);
    }
    // create sampler
    {
        vk::SamplerCreateInfo createInfo = {};
        createInfo.setMagFilter(vk::Filter::eLinear)
            .setMinFilter(vk::Filter::eLinear)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat)
            .setMinLod(-1000.0f)
            .setMaxLod(1000.0f)
            .setMaxAnisotropy(1.0f);

        sampler_ = device.createSampler(createInfo);
    }
    // descriptor set
    descriptorSet_ = (vk::DescriptorSet)ImGui_ImplVulkan_AddTexture(sampler_, imageView_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Image::set(const void* data) {
    auto device = Application::getDevice();
    size_t size = width_ * height_ * Utils::BytesPerPixel(format_);

    if (!staging_) {
        // create staging buffer
        {
            vk::BufferCreateInfo createInfo = {};
            createInfo.setSize(size)
                .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
                .setSharingMode(vk::SharingMode::eExclusive);
            staging_ = device.createBuffer(createInfo);

            vk::MemoryRequirements req = device.getBufferMemoryRequirements(staging_);
            vk::MemoryAllocateInfo allocateInfo = {};
            allocateInfo.setAllocationSize(req.size)
                .setMemoryTypeIndex(Utils::getMemoryType(vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent, req.memoryTypeBits));
            stagingMemory_ = device.allocateMemory(allocateInfo);
            device.bindBufferMemory(staging_, stagingMemory_, 0);
        }
        // create buffer
        {
            vk::BufferCreateInfo createInfo = {};
            createInfo.setSize(size)
                .setUsage(vk::BufferUsageFlagBits::eTransferDst)
                .setSharingMode(vk::SharingMode::eExclusive);
            buffer_ = device.createBuffer(createInfo);

            vk::MemoryRequirements req = device.getBufferMemoryRequirements(buffer_);
            vk::MemoryAllocateInfo allocateInfo = {};
            allocateInfo.setAllocationSize(req.size)
                .setMemoryTypeIndex(Utils::getMemoryType(vk::MemoryPropertyFlagBits::eDeviceLocal, req.memoryTypeBits));
            bufferMemory_ = device.allocateMemory(allocateInfo);
            device.bindBufferMemory(buffer_, bufferMemory_, 0);
        }
    }

    // upload buffer
    void* map = device.mapMemory(stagingMemory_, 0, size);
    memcpy(map, data, size);
    auto cmdbuf = Application::allocateSingleUse();
    vk::BufferCopy regions = {};
    regions.setSrcOffset(0)
        .setDstOffset(0)
        .setSize(size);
    cmdbuf.copyBuffer(staging_, buffer_, regions);
    Application::freeSingleUse(cmdbuf);
    device.unmapMemory(stagingMemory_);

    // copy to image
    cmdbuf = Application::allocateSingleUse();
    vk::ImageMemoryBarrier barrier = {};
    barrier.setSrcAccessMask({})
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setOldLayout(vk::ImageLayout::eUndefined)
        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(image_)
        .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    cmdbuf.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

    vk::BufferImageCopy region = {};
    region.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
        .setImageExtent({width_, height_, 1});
    cmdbuf.copyBufferToImage(staging_, image_, vk::ImageLayout::eTransferDstOptimal, 1, &region);

    vk::ImageMemoryBarrier barrier2 = {};
    barrier2.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(image_)
        .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    cmdbuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier2);
    Application::freeSingleUse(cmdbuf);
}

void Image::resize(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
    reset();
    create(width_ * height_ * Utils::BytesPerPixel(format_));
}

void Image::reset() {
    Application::submitResourceFree([sampler = sampler_, imageView = imageView_, image = image_, imageMemory = imageMemory_, buffer = buffer_, staging = staging_, bufferMemory = bufferMemory_, stagingMemory = stagingMemory_]() {
        auto device = Application::getDevice();
        device.destroySampler(sampler);
        device.destroyImageView(imageView);
        device.destroyImage(image);
        device.freeMemory(imageMemory);
        device.destroyBuffer(buffer);
        device.freeMemory(bufferMemory);
        device.destroyBuffer(staging);
        device.freeMemory(stagingMemory);
    });

    sampler_ = nullptr;
    imageView_ = nullptr;
    image_ = nullptr;
    imageMemory_ = nullptr;
    buffer_ = nullptr;
    bufferMemory_ = nullptr;
    staging_ = nullptr;
    stagingMemory_ = nullptr;
}