#pragma once

#include "utils/enums.hpp"
#include <vulkan/vulkan.hpp>

namespace wen {

// SrcType -> DstType
template <typename DstType, typename SrcType>
DstType convert(SrcType src);

// create imageView
vk::ImageView createImageView(
    vk::Image image, vk::Format format, vk::ImageAspectFlags aspect,
    uint32_t levelCount, uint32_t layerCount = 1, vk::ImageViewType viewType = vk::ImageViewType::e2D
);

// read shader file
std::vector<char> readFile(const std::string& name);

// find memory type
uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

struct TransitionInfo {
    vk::ImageLayout layout;
    vk::AccessFlags access;
    vk::PipelineStageFlags stage;
};
void transitionImageLayout(vk::Image image, vk::ImageAspectFlagBits aspect, uint32_t mipLevels,
                           const TransitionInfo& src, const TransitionInfo& dst);

vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
vk::Format findDepthFormat();

SampleCount getMaxUsableSampleCount();

vk::DeviceAddress getBufferAddress(vk::Buffer buffer);
vk::DeviceAddress getAccelerationStructureAddress(vk::AccelerationStructureKHR as);

} // namespace wen