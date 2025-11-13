#include "../include/logger.hpp"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef USE_POSTGRESQL_ELOG
extern "C" {
#include <postgres.h>
#include <utils/elog.h>
}
#endif

namespace pg_ai::logger {

LogLevel Logger::current_level_ = LogLevel::INFO_LEVEL;
bool Logger::use_postgresql_elog_ = false;

void Logger::setLevel(LogLevel level) { current_level_ = level; }

void Logger::setUsePostgreSQLElog(bool use_elog) { use_postgresql_elog_ = use_elog; }

void Logger::debug(const std::string& message) { log(LogLevel::DEBUG_LEVEL, message); }

void Logger::info(const std::string& message) { log(LogLevel::INFO_LEVEL, message); }

void Logger::warning(const std::string& message) { log(LogLevel::WARNING_LEVEL, message); }

void Logger::error(const std::string& message) { log(LogLevel::ERROR_LEVEL, message); }

void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) {
        return;
    }

    writeLog(level, message);
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG_LEVEL:
            return "DEBUG";
        case LogLevel::INFO_LEVEL:
            return "INFO";
        case LogLevel::WARNING_LEVEL:
            return "WARNING";
        case LogLevel::ERROR_LEVEL:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void Logger::writeLog(LogLevel level, const std::string& message) {
    if (use_postgresql_elog_) {
        // Use PostgreSQL's logging system when running in PostgreSQL
#ifdef USE_POSTGRESQL_ELOG
        switch (level) {
            case LogLevel::DEBUG_LEVEL:
                ereport(DEBUG1, (errmsg("[pg_ai_query] %s", message.c_str())));
                break;
            case LogLevel::INFO_LEVEL:
                ereport(INFO, (errmsg("[pg_ai_query] %s", message.c_str())));
                break;
            case LogLevel::WARNING_LEVEL:
                ereport(WARNING, (errmsg("[pg_ai_query] %s", message.c_str())));
                break;
            case LogLevel::ERROR_LEVEL:
                ereport(LOG, (errmsg("[pg_ai_query] ERROR: %s", message.c_str())));
                break;
        }
#endif
    } else {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);

        std::ostringstream timestamp;
        timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        std::cout << "[" << timestamp.str() << "] "
                  << "[pg_ai_query] "
                  << "[" << levelToString(level) << "] " << message << std::endl;
    }
}

}  // namespace pg_ai::logger