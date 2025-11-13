#include "../include/logger.hpp"

namespace pg_ai::logger {

// Static flag to control logging, set by config manager
static bool logging_enabled = false;

void Logger::setLoggingEnabled(bool enabled) {
  logging_enabled = enabled;
}

void Logger::debug(const std::string& message) {
  if (!logging_enabled)
    return;
  ereport(DEBUG1, (errmsg("[pg_ai_query] %s", message.c_str())));
}

void Logger::info(const std::string& message) {
  if (!logging_enabled)
    return;
  ereport(INFO, (errmsg("[pg_ai_query] %s", message.c_str())));
}

void Logger::warning(const std::string& message) {
  if (!logging_enabled)
    return;
  ereport(WARNING, (errmsg("[pg_ai_query] %s", message.c_str())));
}

void Logger::error(const std::string& message) {
  if (!logging_enabled)
    return;
  ereport(LOG, (errmsg("[pg_ai_query] ERROR: %s", message.c_str())));
}

}  // namespace pg_ai::logger