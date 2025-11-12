#include "../include/query_generator.hpp"
#include "../include/logger.hpp"

#include <ai/openai.h>
#include <regex>
#include <sstream>
#include <algorithm>

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
                pg_ai::Logger::info(std::string("Using environment API key: ") + std::string(env_api_key).substr(0, 10) + "...");
                api_key = env_api_key;
            } else {
                pg_ai::Logger::warning("No OPENAI_API_KEY environment variable found");
                return {.success = false, .error_message = "OpenAI API key required. Pass as 4th parameter or set OPENAI_API_KEY environment variable."};
            }
        }

        std::string prompt = buildPrompt(request);

        ai::Client client = [&]() {
            try {
                pg_ai::Logger::info("Creating OpenAI client");
                return ai::openai::create_client(api_key);
            } catch (const std::exception& e) {
                pg_ai::Logger::error("Failed to create OpenAI client: " + std::string(e.what()));
                throw std::runtime_error("Failed to create OpenAI client: " + std::string(e.what()));
            }
        }();

        // Use AI SDK directly - construct with parameters like examples
        ai::GenerateOptions options(
            ai::openai::models::kGpt4o,
            "You are a PostgreSQL query generator. Generate safe, read-only SQL queries from natural language.",
            prompt
        );

        auto result = client.generate_text(options);

        if (!result) {
            return {.success = false, .error_message = "AI API error: " + result.error_message()};
        }

        if (result.text.empty()) {
            return {.success = false, .error_message = "Empty response from AI service"};
        }

        // Extract SQL from response
        std::string generated_query = extractSQLFromResponse(result.text);

        // Validate safety
        // if (!isSafeQuery(generated_query)) {
        //     return {.success = false, .error_message = "Generated query failed safety validation"};
        // }

        return {
            .generated_query = generated_query,
            .explanation = "Query generated using GPT-4",
            .success = true,
            .error_message = ""
        };

    } catch (const std::exception& e) {
        return {.success = false, .error_message = std::string("Exception: ") + e.what()};
    }
}

std::string QueryGenerator::buildPrompt(const QueryRequest& request) {
    std::ostringstream prompt;

    prompt << "Generate a PostgreSQL SELECT query for this request:\n\n";
    prompt << "Request: " << request.natural_language << "\n";

    if (!request.table_name.empty()) {
        prompt << "Table: " << request.table_name << "\n";
    }

    if (!request.schema_context.empty()) {
        prompt << "Schema info:\n" << request.schema_context << "\n";
    }

    prompt << "\nRequirements:\n";
    prompt << "- ONLY SELECT queries (no INSERT, UPDATE, DELETE, DROP, etc.)\n";
    prompt << "- Use proper PostgreSQL syntax\n";
    prompt << "- Include appropriate WHERE clauses for performance\n";
    prompt << "- Add LIMIT when appropriate\n";
    prompt << "- Return only the SQL query, no explanations\n";

    return prompt.str();
}

std::string QueryGenerator::extractSQLFromResponse(const std::string& response) {
    // Look for SQL code blocks first
    std::regex sql_block_regex(R"(```(?:sql)?\s*([\s\S]*?)\s*```)", std::regex::icase);
    std::smatch match;

    if (std::regex_search(response, match, sql_block_regex)) {
        return match[1].str();
    }

    // Look for SELECT statements
    std::regex select_regex(R"((SELECT[\s\S]*?)(?:\n\n|\Z))", std::regex::icase);
    if (std::regex_search(response, match, select_regex)) {
        return match[1].str();
    }

    // Return response as-is if no structured SQL found
    return response;
}

bool QueryGenerator::isSafeQuery(const std::string& query) {
    if (query.empty()) return false;

    std::string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

    // Must contain SELECT
    if (lower_query.find("select") == std::string::npos) {
        return false;
    }

    // Check for dangerous operations
    std::vector<std::string> forbidden = {
        "insert", "update", "delete", "drop", "create", "alter",
        "truncate", "grant", "revoke", "exec", "execute"
    };

    for (const auto& keyword : forbidden) {
        if (lower_query.find(keyword) != std::string::npos) {
            return false;
        }
    }

    return true;
}

} // namespace pg_ai