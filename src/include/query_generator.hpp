#pragma once

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
    bool success;
    std::string error_message;
};

class QueryGenerator {
public:
    static QueryResult generateQuery(const QueryRequest& request);

private:
    static std::string buildPrompt(const QueryRequest& request);
    static std::string extractSQLFromResponse(const std::string& response);
    static bool isSafeQuery(const std::string& query);
};

}