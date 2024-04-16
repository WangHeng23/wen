#include "core/logger.hpp"
#include "utils/utils.hpp"

namespace wen {

Logger* logger = nullptr;

Logger::Logger(Level level) {
    logger_ = spdlog::stdout_color_mt("wen");
    logger_->set_level(convert<spdlog::level::level_enum>(level));
}

Logger::~Logger() {
    spdlog::drop(logger_->name());
    logger_.reset();
    logger_ = nullptr;
}

void Logger::setLevel(Level level) {
    logger_->set_level(convert<spdlog::level::level_enum>(level));
}

} // namespace wen