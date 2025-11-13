#include "./include/utils.hpp"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "./include/logger.hpp"

namespace pg_ai::utils {

std::pair<bool, std::string> read_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        logger::Logger::error("Failed to open file: " + filepath);
        return {false, {}};
    }

    const auto size = file.tellg();
    if (size == -1) {
        logger::Logger::error("Invalid file size: " + filepath);
        return {false, {}};
    }

    file.seekg(0, std::ios::beg);

    std::string content(static_cast<std::size_t>(size), '\0');
    if (size > 0) {
        if (!file.read(&content[0], static_cast<std::streamsize>(size))) {
            logger::Logger::error("Failed to read file: " + filepath);
            return {false, {}};
        }
    }

    return {true, std::move(content)};
}

std::string read_file_or_throw(const std::string& filepath) {
    auto [success, content] = read_file(filepath);
    if (!success) {
        throw std::runtime_error("Failed to read file: " + filepath);
    }
    return std::move(content);
}
}  // namespace pg_ai::utils