#include "../include/query_generator.hpp"

#include <ai/openai.h>
#include <ai/anthropic.h>

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <regex>
#include <sstream>
#include <vector>

#include "../include/config.hpp"
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

        const auto& cfg = config::ConfigManager::getConfig();

        std::string api_key = request.api_key;
        std::string api_key_source = "parameter";
        std::string provider_preference = request.provider;

        config::Provider selected_provider;
        const config::ProviderConfig* provider_config = nullptr;

        if (provider_preference == "openai") {
            selected_provider = config::Provider::OPENAI;
            provider_config = config::ConfigManager::getProviderConfig(config::Provider::OPENAI);
            logger::Logger::info("Explicit OpenAI provider selection from parameter");

            if (api_key.empty() && provider_config && !provider_config->api_key.empty()) {
                api_key = provider_config->api_key;
                api_key_source = "openai_config";
                logger::Logger::info("Using OpenAI API key from configuration");
            }
        } else if (provider_preference == "anthropic") {
            selected_provider = config::Provider::ANTHROPIC;
            provider_config = config::ConfigManager::getProviderConfig(config::Provider::ANTHROPIC);
            logger::Logger::info("Explicit Anthropic provider selection from parameter");

            if (api_key.empty() && provider_config && !provider_config->api_key.empty()) {
                api_key = provider_config->api_key;
                api_key_source = "anthropic_config";
                logger::Logger::info("Using Anthropic API key from configuration");
            }
        } else {
            if (api_key.empty()) {
                const auto* openai_config = config::ConfigManager::getProviderConfig(config::Provider::OPENAI);
                if (openai_config && !openai_config->api_key.empty()) {
                    logger::Logger::info("Auto-selecting OpenAI provider based on configuration");
                    selected_provider = config::Provider::OPENAI;
                    provider_config = openai_config;
                    api_key = openai_config->api_key;
                    api_key_source = "openai_config";
                } else {
                    const auto* anthropic_config = config::ConfigManager::getProviderConfig(config::Provider::ANTHROPIC);
                    if (anthropic_config && !anthropic_config->api_key.empty()) {
                        logger::Logger::info("Auto-selecting Anthropic provider based on configuration");
                        selected_provider = config::Provider::ANTHROPIC;
                        provider_config = anthropic_config;
                        api_key = anthropic_config->api_key;
                        api_key_source = "anthropic_config";
                    } else {
                        logger::Logger::warning("No API key found in config");
                        return {.success = false,
                                .error_message = "API key required. Pass as 4th parameter or set OpenAI or Anthropic API key in ~/.pg_ai.config."};
                    }
                }
            } else {
                selected_provider = config::Provider::OPENAI;
                provider_config = config::ConfigManager::getProviderConfig(config::Provider::OPENAI);
                logger::Logger::info("Auto-selecting OpenAI provider (API key provided, no provider specified)");
            }
        }

        if (api_key.empty()) {
            std::string provider_name = config::ConfigManager::providerToString(selected_provider);
            return {.success = false,
                    .error_message = "No API key available for " + provider_name + " provider. Please provide API key as parameter or configure it in ~/.pg_ai.config."};
        }

        std::string system_prompt = prompts::SYSTEM_PROMPT;

        std::string prompt = buildPrompt(request);

        config::Provider provider = selected_provider;

        ai::Client client;
        std::string model_name;

        try {
            if (provider == config::Provider::OPENAI) {
                logger::Logger::info("Creating OpenAI client");
                client = ai::openai::create_client(api_key);
                model_name = (provider_config && !provider_config->default_model.name.empty()) ?
                    provider_config->default_model.name : "gpt-4o";
            } else if (provider == config::Provider::ANTHROPIC) {
                logger::Logger::info("Creating Anthropic client");
                client = ai::anthropic::create_client(api_key);
                model_name = (provider_config && !provider_config->default_model.name.empty()) ?
                    provider_config->default_model.name : "claude-3-5-sonnet-20241022";
            } else {
                logger::Logger::warning("Unknown provider, defaulting to OpenAI");
                client = ai::openai::create_client(api_key);
                model_name = "gpt-4o";
            }

            logger::Logger::info("Using " + config::ConfigManager::providerToString(provider) +
                               " provider with model: " + model_name);
        } catch (const std::exception& e) {
            logger::Logger::error("Failed to create " + config::ConfigManager::providerToString(provider) +
                                " client: " + std::string(e.what()));
            throw std::runtime_error("Failed to create AI client: " + std::string(e.what()));
        }

        ai::GenerateOptions options(model_name, system_prompt, prompt);

        const config::ModelConfig* model_config = config::ConfigManager::getModelConfig(model_name);
        if (model_config) {
            options.max_tokens = model_config->max_tokens;
            options.temperature = model_config->temperature;
            logger::Logger::info("Using model: " + model_name + " with max_tokens=" + std::to_string(model_config->max_tokens) +
                               ", temperature=" + std::to_string(model_config->temperature));
        } else {
            logger::Logger::info("Using model: " + model_name + " with default settings");
        }

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

}  // namespace pg_ai