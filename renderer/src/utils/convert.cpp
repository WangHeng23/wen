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

} // namespace wen