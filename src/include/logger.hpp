#pragma once

#include <string>
#include <memory>

namespace pg_ai {

enum class LogLevel {
    DEBUG_LEVEL = 0,
    INFO_LEVEL = 1,
    WARNING_LEVEL = 2,
    ERROR_LEVEL = 3
};

class Logger {
public:
    // Static interface for easy access throughout the application
    static void setLevel(LogLevel level);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);

    // Direct logging with level
    static void log(LogLevel level, const std::string& message);

    // Enable/disable PostgreSQL elog integration
    static void setUsePostgreSQLElog(bool use_elog);

private:
    static LogLevel current_level_;
    static bool use_postgresql_elog_;

    static const char* levelToString(LogLevel level);
    static void writeLog(LogLevel level, const std::string& message);
};

// Convenience macros for easier logging
#define PG_AI_LOG_DEBUG(msg) pg_ai::Logger::debug(msg)
#define PG_AI_LOG_INFO(msg) pg_ai::Logger::info(msg)
#define PG_AI_LOG_WARNING(msg) pg_ai::Logger::warning(msg)
#define PG_AI_LOG_ERROR(msg) pg_ai::Logger::error(msg)

} // namespace pg_ai