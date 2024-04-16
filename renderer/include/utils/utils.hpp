#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

// SrcType -> DstType
template <typename DstType, typename SrcType>
DstType convert(SrcType src);

} // namespace wen