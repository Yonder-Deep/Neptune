#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "types.hpp"   // LogSource, to_string(LogSource)

// This is a file defining how the logger object will function. It is a
// wrapper of spdlog. There is no logger class, the methods exist as free
// functions in a namespace to avoid complications with passing logger
// objects back and forth between different threads, etc.

// call log_init to start the logger. Before initialization, logging calls are safe no-ops. 
// After initialization, logs will be sent to both console and file with the specified levels. 
// The logger is thread-safe.

// The file sink is a rotating logger that creates a new file
// when the current one exceeds 5MB, keeping up to 3 old files.

// We have two sinks, one to the console and one to a rotating file.
// Each sink can have its own log level threshold.
// A sink prints a message only if message_level >= sink_level. 
// So you can have debug logs go to log file but only info+ go to console, base station, etc.

// Example usage:
//   log_init("usv.log", LogLevel::Info, LogLevel::Debug);
//   log_info(LogSource::MAIN, "This is an info message");
//   log_debug(LogSource::NAV, "This is a debug message");

namespace neptune {

//------------------------------------------------------------------------------
// Log level (local to logging system)
//------------------------------------------------------------------------------

enum class LogLevel { // in ascending order of severity. >= threshold means "log this message"
    Debug,
    Info,
    Warn,
    Error,
    Critical
};

//------------------------------------------------------------------------------
// Internal details
//------------------------------------------------------------------------------

namespace detail {

inline std::shared_ptr<spdlog::logger> logger;
inline std::once_flag init_flag;

inline spdlog::level::level_enum to_spdlog_level(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::Debug:     return spdlog::level::debug;
        case LogLevel::Info:      return spdlog::level::info;
        case LogLevel::Warn:      return spdlog::level::warn;
        case LogLevel::Error:     return spdlog::level::err;
        case LogLevel::Critical:  return spdlog::level::critical;
        default:                  return spdlog::level::info;
    }
}

} // namespace detail

//------------------------------------------------------------------------------
// Initialization / shutdown
// init is idempotent, so calling init multiple times won't cause issues.
// Shutdown will flush logs and destroy the logger object.
//------------------------------------------------------------------------------

inline void log_init(const std::string& log_file,
                     LogLevel console_level,
                     LogLevel file_level)
{
    std::call_once(detail::init_flag, [&]() {
        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(detail::to_spdlog_level(console_level));

        auto file_sink =
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                log_file,
                5 * 1024 * 1024, // 5MB before rotating
                3); // max 3 files before rotating
        file_sink->set_level(detail::to_spdlog_level(file_level));

        detail::logger = std::make_shared<spdlog::logger>(
            "neptune",
            spdlog::sinks_init_list{console_sink, file_sink});

        // Base pattern (SOURCE injected per call)
        detail::logger->set_pattern("[%H:%M:%S.%f] [%^%l%$] %v");

        detail::logger->set_level(spdlog::level::trace);
        detail::logger->flush_on(spdlog::level::warn);

        spdlog::register_logger(detail::logger);
    });
}


// shutdown logger and destroy logger object.
inline void log_shutdown()
{
    if (detail::logger) {
        detail::logger->flush();
        spdlog::drop(detail::logger->name());
        detail::logger.reset();
    }
    spdlog::shutdown();
}

//------------------------------------------------------------------------------
// Core logging API
// calls spdlog with the appropriate level and injects the SOURCE into the message format.
//------------------------------------------------------------------------------

inline void log_message(LogSource source,
                        LogLevel level,
                        std::string_view message)
{
    if (!detail::logger) {
        return; // safe no-op if not initialized
    }

    // Inject SOURCE into message without extra logger creation
    detail::logger->log(
        detail::to_spdlog_level(level),
        "[{}] {}",
        to_string(source),
        message);
}

//------------------------------------------------------------------------------
// Convenience helpers
// in practice, call these methods to log at different levels instead of log_message directly.
// provide source and message
//------------------------------------------------------------------------------

inline void log_debug(LogSource src, std::string_view msg) {
    log_message(src, LogLevel::Debug, msg);
}

inline void log_info(LogSource src, std::string_view msg) {
    log_message(src, LogLevel::Info, msg);
}

inline void log_warn(LogSource src, std::string_view msg) {
    log_message(src, LogLevel::Warn, msg);
}

inline void log_error(LogSource src, std::string_view msg) {
    log_message(src, LogLevel::Error, msg);
}

inline void log_critical(LogSource src, std::string_view msg) {
    log_message(src, LogLevel::Critical, msg);
}

} // namespace neptune
