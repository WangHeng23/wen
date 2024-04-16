#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace wen {

class Logger final {
public:
    enum class Level { trace, debug, info, warn, error, critical };

public:
    Logger(Level level);
    ~Logger();

    void setLevel(Level level);

    template <typename... Args>
    void trace(Args&&... args) { logger_->trace(std::forward<Args>(args)...); }

    template <typename... Args>
    void debug(Args&&... args) { logger_->debug(std::forward<Args>(args)...); }

    template <typename... Args>
    void info(Args&&... args) { logger_->info(std::forward<Args>(args)...); }

    template <typename... Args>
    void warn(Args&&... args) { logger_->warn(std::forward<Args>(args)...); }

    template <typename... Args>
    void error(Args&&... args) { logger_->error(std::forward<Args>(args)...); }

    template <typename... Args>
    void critical(Args&&... args) { logger_->critical(std::forward<Args>(args)...); }

private:
    std::shared_ptr<spdlog::logger> logger_;
};

extern Logger* logger;

} // namespace wen

#define WEN_TRACE(...) ::wen::logger->trace(__VA_ARGS__);
#define WEN_DEBUG(...) ::wen::logger->debug(__VA_ARGS__);
#define WEN_INFO(...) ::wen::logger->info(__VA_ARGS__);
#define WEN_WARN(...) ::wen::logger->warn(__VA_ARGS__);
#define WEN_ERROR(...) ::wen::logger->error(__VA_ARGS__);
#define WEN_CRITICAL(...) ::wen::logger->critical(__VA_ARGS__);

#define WEN_ASSERT(x, ...)                                  \
    if (!(x)) {                                             \
        WEN_CRITICAL("Assertion Failed: {0}", __VA_ARGS__); \
        std::abort();                                       \
    }