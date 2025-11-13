#pragma once
#include <string>
#include <utility>

namespace pg_ai::utils {

/**
 * @brief Reads the entire file into a string.
 *
 * @return std::pair<bool, std::string>
 *         - first  = true  → success, second = file contents
 *         - first  = false → error,   second = empty string
 */
std::pair<bool, std::string> read_file(const std::string& filepath);

/**
 * @brief Reads file and throws std::runtime_error on failure.
 */
std::string read_file_or_throw(const std::string& filepath);
}  // namespace pg_ai::utils