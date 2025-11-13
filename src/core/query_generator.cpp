#include "../include/query_generator.hpp"

#include <ai/openai.h>

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <vector>

#include "../include/logger.hpp"
#include "../include/prompts.hpp"
#include "../include/utils.hpp"

using namespace pg_ai::logger;

namespace pg_ai {

QueryResult QueryGenerator::generateQuery(const QueryRequest& request) {
    try {
        if (request.natural_language.empty()) {
            return {.success = false, .error_message = "Natural language query cannot be empty"};
        }

        std::string api_key = request.api_key;
        if (api_key.empty()) {
            const char* env_api_key = std::getenv("OPENAI_API_KEY");
            if (env_api_key) {
                logger::Logger::info(std::string("Using environment API key: ") +
                                     std::string(env_api_key).substr(0, 10) + "...");
                api_key = env_api_key;
            } else {
                logger::Logger::warning("No OPENAI_API_KEY environment variable found");
                return {.success = false,
                        .error_message =
                            "OpenAI API key required. Pass as 4th parameter or set OPENAI_API_KEY "
                            "environment variable."};
            }
        }

        std::string system_prompt = prompts::SYSTEM_PROMPT;

        std::string prompt = buildPrompt(request);

        ai::Client client = [&]() {
            try {
                logger::Logger::info("Creating OpenAI client");
                return ai::openai::create_client(api_key);
            } catch (const std::exception& e) {
                logger::Logger::error("Failed to create OpenAI client: " + std::string(e.what()));
                throw std::runtime_error("Failed to create OpenAI client: " +
                                         std::string(e.what()));
            }
        }();

        ai::GenerateOptions options(ai::openai::models::kGpt4o, system_prompt, prompt);

        auto result = client.generate_text(options);

        if (!result) {
            return {.success = false, .error_message = "AI API error: " + result.error_message()};
        }

        if (result.text.empty()) {
            return {.success = false, .error_message = "Empty response from AI service"};
        }

        nlohmann::json j = extractSQLFromResponse(result.text);
        std::string sql = j.value("sql", "");
        if (sql.empty()) {
            return {.success = false, .error_message = "No SQL found in model response"};
        }

        std::vector<std::string> warnings_vec;
        try {
            if (j.contains("warnings")) {
                if (j["warnings"].is_array()) {
                    warnings_vec = j["warnings"].get<std::vector<std::string>>();
                } else if (j["warnings"].is_string()) {
                    warnings_vec.push_back(j["warnings"].get<std::string>());
                }
            }
        } catch (...) {
        }

        return {.generated_query = sql,
                .explanation = j.value("explaination", ""),
                .warnings = warnings_vec,
                .row_limit_applied = j.value("row_limit_applied", false),
                .suggested_visualization = j.value("suggested_visualization", "table"),
                .success = true,
                .error_message = ""};
    } catch (const std::exception& e) {
        return {.success = false, .error_message = std::string("Exception: ") + e.what()};
    }
}

std::string QueryGenerator::buildPrompt(const QueryRequest& request) {
    std::ostringstream prompt;

    prompt << "Generate a PostgreSQL query for this request:\n\n";
    prompt << "Request: " << request.natural_language << "\n";

    if (!request.table_name.empty()) {
        prompt << "Table: " << request.table_name << "\n";
    }

    if (!request.schema_context.empty()) {
        prompt << "Schema info:\n" << request.schema_context << "\n";
    }

    // prompt << "\nRequirements:\n";
    // prompt << "- ONLY SELECT queries (no INSERT, UPDATE, DELETE, DROP, etc.)\n";
    // prompt << "- Use proper PostgreSQL syntax\n";
    // prompt << "- Include appropriate WHERE clauses for performance\n";
    // prompt << "- Add LIMIT when appropriate\n";
    // prompt << "- Return only the SQL query, no explanations\n";

    return prompt.str();
}

nlohmann::json QueryGenerator::extractSQLFromResponse(const std::string& text) {
    std::regex json_block(R"(```(?:json)?\s*(\{[\s\S]*?\})\s*```)", std::regex::icase);
    std::smatch match;

    if (std::regex_search(text, match, json_block)) {
        try {
            return nlohmann::json::parse(match[1].str());
        } catch (...) {
        }
    }

    try {
        return nlohmann::json::parse(text);
    } catch (...) {
    }

    // Fallback
    return {{"sql", text}, {"explanation", "Raw LLM output (no JSON detected)"}};
}

// bool QueryGenerator::isSafeQuery(const std::string& query) {
//     if (query.empty()) return false;

//     std::string lower_query = query;
//     std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

//     // Must contain SELECT
//     if (lower_query.find("select") == std::string::npos) {
//         return false;
//     }

//     // Check for dangerous operations
//     std::vector<std::string> forbidden = {
//         "insert", "update", "delete", "drop", "create", "alter",
//         "truncate", "grant", "revoke", "exec", "execute"
//     };

//     for (const auto& keyword : forbidden) {
//         if (lower_query.find(keyword) != std::string::npos) {
//             return false;
//         }
//     }

//     return true;
// }

}  // namespace pg_ai