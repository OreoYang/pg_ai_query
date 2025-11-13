#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace pg_ai {

struct QueryRequest {
    std::string natural_language;
    std::string table_name;
    std::string schema_context;
    std::string api_key;
};

struct QueryResult {
    std::string generated_query;
    std::string explanation;
    std::vector<std::string> warnings;
    bool row_limit_applied;
    std::string suggested_visualization;
    bool success;
    std::string error_message;
};

class QueryGenerator {
   public:
    static QueryResult generateQuery(const QueryRequest& request);

   private:
    static std::string buildPrompt(const QueryRequest& request);
    static nlohmann::json extractSQLFromResponse(const std::string& response);
};

}  // namespace pg_ai