extern "C" {
#include <postgres.h>
#include <fmgr.h>
#include <funcapi.h>
#include <utils/builtins.h>
#include <utils/elog.h>
}

#include "include/query_generator.hpp"

extern "C" {
PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_generate_query);

/**
 * pg_generate_query(natural_language_query text, table_name text DEFAULT '', schema_context text
 * DEFAULT '', api_key text DEFAULT NULL, provider text DEFAULT 'auto')
 *
 * Generates a SQL query from natural language input using specified AI provider
 * Provider options: 'openai', 'anthropic', 'auto' (auto-select based on config)
 */
Datum pg_generate_query(PG_FUNCTION_ARGS) {
    try {
        text* nl_query_arg = PG_GETARG_TEXT_PP(0);
        text* table_name_arg = PG_ARGISNULL(1) ? nullptr : PG_GETARG_TEXT_PP(1);
        text* schema_context_arg = PG_ARGISNULL(2) ? nullptr : PG_GETARG_TEXT_PP(2);
        text* api_key_arg = PG_ARGISNULL(3) ? nullptr : PG_GETARG_TEXT_PP(3);
        text* provider_arg = PG_ARGISNULL(4) ? nullptr : PG_GETARG_TEXT_PP(4);

        std::string nl_query = text_to_cstring(nl_query_arg);
        std::string table_name = table_name_arg ? text_to_cstring(table_name_arg) : "";
        std::string schema_context = schema_context_arg ? text_to_cstring(schema_context_arg) : "";
        std::string api_key = api_key_arg ? text_to_cstring(api_key_arg) : "";
        std::string provider = provider_arg ? text_to_cstring(provider_arg) : "auto";

        pg_ai::QueryRequest request{.natural_language = nl_query,
                                    .table_name = table_name,
                                    .schema_context = schema_context,
                                    .api_key = api_key,
                                    .provider = provider};

        auto result = pg_ai::QueryGenerator::generateQuery(request);

        if (!result.success) {
            ereport(ERROR, (errcode(ERRCODE_EXTERNAL_ROUTINE_EXCEPTION),
                            errmsg("Query generation failed: %s", result.error_message.c_str())));
        }

        ereport(INFO, (errmsg("Generated SQL query: %s", result.explanation.c_str())));

        PG_RETURN_TEXT_P(cstring_to_text(result.generated_query.c_str()));
    } catch (const std::exception& e) {
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Internal error: %s", e.what())));
        PG_RETURN_NULL();
    }
}
}