#pragma once

#include <vulkan/vulkan.hpp>

namespace wen {

enum class AttachmentType {
    eColor,
    eDepth,
};

enum class Topology {
    ePointList,
    eLineList,
    eLineStrip,
    eTriangleList,
    eTriangleStrip,
    eTriangleFan,
};

enum class PolygonMode {
    eFill,
    eLine,
    ePoint,
};

enum class CullMode {
    eNone,
    eFront,
    eBack,
    eFrontAndBack,
};

enum class FrontFace {
    eClockwise,
    eCounterClockwise,
};

} // namespace wen