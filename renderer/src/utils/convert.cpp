#include "utils/utils.hpp"
#include "utils/enums.hpp"
#include "core/logger.hpp"

namespace wen {

template <>
spdlog::level::level_enum convert<spdlog::level::level_enum>(Logger::Level level) {
    switch (level) {
        case Logger::Level::trace : return spdlog::level::level_enum::trace;
        case Logger::Level::debug : return spdlog::level::level_enum::debug;
        case Logger::Level::info : return spdlog::level::level_enum::info;
        case Logger::Level::warn : return spdlog::level::level_enum::warn;
        case Logger::Level::error : return spdlog::level::level_enum::err;
        case Logger::Level::critical : return spdlog::level::level_enum::critical;
    }
}

template <>
vk::PrimitiveTopology convert<vk::PrimitiveTopology>(Topology topology) {
    switch (topology) {
        case Topology::ePointList: return vk::PrimitiveTopology::ePointList;
        case Topology::eLineList: return vk::PrimitiveTopology::eLineList;
        case Topology::eLineStrip: return vk::PrimitiveTopology::eLineStrip;
        case Topology::eTriangleList: return vk::PrimitiveTopology::eTriangleList;
        case Topology::eTriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
        case Topology::eTriangleFan: return vk::PrimitiveTopology::eTriangleFan;
    }
}

template <>
vk::PolygonMode convert<vk::PolygonMode>(PolygonMode mode) {
    switch (mode) {
        case PolygonMode::eFill: return vk::PolygonMode::eFill;
        case PolygonMode::eLine: return vk::PolygonMode::eLine;
        case PolygonMode::ePoint: return vk::PolygonMode::ePoint;
    }
}

template <>
vk::CullModeFlags convert<vk::CullModeFlags>(CullMode mode) {
    switch (mode) {
        case CullMode::eNone: return vk::CullModeFlagBits::eNone;
        case CullMode::eFront: return vk::CullModeFlagBits::eFront;
        case CullMode::eBack: return vk::CullModeFlagBits::eBack;
        case CullMode::eFrontAndBack: return vk::CullModeFlagBits::eFrontAndBack;
    }
}

template <>
vk::FrontFace convert<vk::FrontFace>(FrontFace mode) {
    switch (mode) {
        case FrontFace::eClockwise: return vk::FrontFace::eClockwise;
        case FrontFace::eCounterClockwise: return vk::FrontFace::eCounterClockwise;
    }
}

template <>
vk::VertexInputRate convert<vk::VertexInputRate>(InputRate rate) {
    switch (rate) {
        case InputRate::eVertex: return vk::VertexInputRate::eVertex;
        case InputRate::eInstance: return vk::VertexInputRate::eInstance;
    }
}

template <>
vk::Format convert<vk::Format>(VertexType type) {
    switch (type) {
        case VertexType::eInt8: return vk::Format::eR8Sint;
        case VertexType::eInt8x2: return vk::Format::eR8G8Sint;
        case VertexType::eInt8x3: return vk::Format::eR8G8B8Sint;
        case VertexType::eInt8x4: return vk::Format::eR8G8B8A8Sint;
        case VertexType::eInt16: return vk::Format::eR16Sint;
        case VertexType::eInt16x2: return vk::Format::eR16G16Sint;
        case VertexType::eInt16x3: return vk::Format::eR16G16B16Sint;
        case VertexType::eInt16x4: return vk::Format::eR16G16B16A16Sint;
        case VertexType::eInt32: return vk::Format::eR32Sint;
        case VertexType::eInt32x2: return vk::Format::eR32G32Sint;
        case VertexType::eInt32x3: return vk::Format::eR32G32B32Sint;
        case VertexType::eInt32x4: return vk::Format::eR32G32B32A32Sint;
        case VertexType::eInt64: return vk::Format::eR64Sint;
        case VertexType::eInt64x2: return vk::Format::eR64G64Sint;
        case VertexType::eInt64x3: return vk::Format::eR64G64B64Sint;
        case VertexType::eInt64x4: return vk::Format::eR64G64B64A64Sint;
        case VertexType::eUint8: return vk::Format::eR8Uint;
        case VertexType::eUint8x2: return vk::Format::eR8G8Uint;
        case VertexType::eUint8x3: return vk::Format::eR8G8B8Uint;
        case VertexType::eUint8x4: return vk::Format::eR8G8B8A8Uint;
        case VertexType::eUint16: return vk::Format::eR16Uint;
        case VertexType::eUint16x2: return vk::Format::eR16G16Uint;
        case VertexType::eUint16x3: return vk::Format::eR16G16B16Uint;
        case VertexType::eUint16x4: return vk::Format::eR16G16B16A16Uint;
        case VertexType::eUint32: return vk::Format::eR32Uint;
        case VertexType::eUint32x2: return vk::Format::eR32G32Uint;
        case VertexType::eUint32x3: return vk::Format::eR32G32B32Uint;
        case VertexType::eUint32x4: return vk::Format::eR32G32B32A32Uint;
        case VertexType::eUint64: return vk::Format::eR64Uint;
        case VertexType::eUint64x2: return vk::Format::eR64G64Uint;
        case VertexType::eUint64x3: return vk::Format::eR64G64B64Uint;
        case VertexType::eUint64x4: return vk::Format::eR64G64B64A64Uint;
        case VertexType::eFloat: return vk::Format::eR32Sfloat;
        case VertexType::eFloat2: return vk::Format::eR32G32Sfloat;
        case VertexType::eFloat3: return vk::Format::eR32G32B32Sfloat;
        case VertexType::eFloat4: return vk::Format::eR32G32B32A32Sfloat;
        case VertexType::eDouble: return vk::Format::eR64Sfloat;
        case VertexType::eDouble2: return vk::Format::eR64G64Sfloat;
        case VertexType::eDouble3: return vk::Format::eR64G64B64Sfloat;
        case VertexType::eDouble4: return vk::Format::eR64G64B64A64Sfloat;
    }
}

template <>
uint32_t convert<uint32_t>(VertexType type) {
    switch (type) {
        case VertexType::eInt8: return 1;
        case VertexType::eInt8x2: return 2;
        case VertexType::eInt8x3: return 3;
        case VertexType::eInt8x4: return 4;
        case VertexType::eInt16: return 2;
        case VertexType::eInt16x2: return 4;
        case VertexType::eInt16x3: return 6;
        case VertexType::eInt16x4: return 8;
        case VertexType::eInt32: return 4;
        case VertexType::eInt32x2: return 8;
        case VertexType::eInt32x3: return 12;
        case VertexType::eInt32x4: return 16;
        case VertexType::eInt64: return 8;
        case VertexType::eInt64x2: return 16;
        case VertexType::eInt64x3: return 24;
        case VertexType::eInt64x4: return 32;
        case VertexType::eUint8: return 1;
        case VertexType::eUint8x2: return 2;
        case VertexType::eUint8x3: return 3;
        case VertexType::eUint8x4: return 4;
        case VertexType::eUint16: return 2;
        case VertexType::eUint16x2: return 4;
        case VertexType::eUint16x3: return 6;
        case VertexType::eUint16x4: return 8;
        case VertexType::eUint32: return 4;
        case VertexType::eUint32x2: return 8;
        case VertexType::eUint32x3: return 12;
        case VertexType::eUint32x4: return 16;
        case VertexType::eUint64: return 8;
        case VertexType::eUint64x2: return 16;
        case VertexType::eUint64x3: return 24;
        case VertexType::eUint64x4: return 32;
        case VertexType::eFloat: return 4;
        case VertexType::eFloat2: return 8;
        case VertexType::eFloat3: return 12;
        case VertexType::eFloat4: return 16;
        case VertexType::eDouble: return 8;
        case VertexType::eDouble2: return 16;
        case VertexType::eDouble3: return 24;
        case VertexType::eDouble4: return 32;
    }
}

} // namespace wen