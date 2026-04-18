#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <thread>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "types.hpp"   // LogSource, OutboundMessage, etc.
#include "time_utils.hpp"  // Time utilities for timestamp generation
#include "../messages/messages.pb.h"  // Protobuf messages
#include "../tasks/thread_task.hpp"  // ThreadTask for enqueue_message()

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
//   set_current_task(my_task);  // Call once per thread
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

// Thread-local pointer to the current task's queue for enqueuing messages.
// Each thread calls set_current_task() at startup to point to its task.
inline thread_local ThreadTask* current_task = nullptr;

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

// Convert LogLevel to string representation
inline std::string log_level_to_string(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::Debug:     return "DEBUG";
        case LogLevel::Info:      return "INFO";
        case LogLevel::Warn:      return "WARN";
        case LogLevel::Error:     return "ERROR";
        case LogLevel::Critical:  return "CRITICAL";
        default:                  return "UNKNOWN";
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
// Task queue management
// Call set_current_task() once per thread to enable log enqueueing
//------------------------------------------------------------------------------

inline void set_current_task(ThreadTask* task)
{
    detail::current_task = task;
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

    // Send to console and file via spdlog
    detail::logger->log(
        detail::to_spdlog_level(level),
        "[{}] {}",
        to_string(source),
        message);

    // Enqueue to task's outbound queue for transmission to base (if info and above)
    if (detail::current_task && level >= LogLevel::Info) {
        try {
            // Create protobuf LogEntry
            neptune::LogEntry log_entry;
            log_entry.set_timestamp_ms(get_timestamp_ms());
            log_entry.set_source(std::string(to_string(source)));
            log_entry.set_level(detail::log_level_to_string(level));
            log_entry.set_message(std::string(message));

            // Wrap in Envelope
            neptune::Envelope envelope;
            envelope.set_type(neptune::Envelope::LOG);
            envelope.set_payload(log_entry.SerializeAsString());
            envelope.set_timestamp_ms(log_entry.timestamp_ms());
            envelope.set_source("neptune");

            // Serialize and enqueue to task
            std::string serialized = envelope.SerializeAsString();
            detail::current_task->enqueue_message(serialized);
        } catch (const std::exception& e) {
            // Silently ignore enqueue errors to avoid breaking the logger
        }
    }
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
