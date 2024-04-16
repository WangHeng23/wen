#include "utils/utils.hpp"
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

} // namespace wen