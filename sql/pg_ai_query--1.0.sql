-- AI Query Generator Extension for PostgreSQL
-- Generates SQL queries from natural language using OpenAI

-- Version 1.0

-- Main function: Generate SQL from natural language
CREATE OR REPLACE FUNCTION pg_generate_query(
    natural_language_query text,
    table_name text DEFAULT '',
    schema_context text DEFAULT '',
    api_key text DEFAULT NULL
)
RETURNS text
AS 'MODULE_PATHNAME', 'pg_generate_query'
LANGUAGE C;

-- Example usage:
-- SELECT pg_generate_query('Show me all users created in the last 7 days', 'users');
-- SELECT pg_generate_query('Count orders by status', 'orders', 'columns: id, user_id, status, created_at');
-- SELECT pg_generate_query('Show me all users', '', '', 'your-api-key-here');

COMMENT ON FUNCTION pg_generate_query(text, text, text, text) IS
'Generate a PostgreSQL SELECT query from natural language description. Pass API key as 4th parameter or set OPENAI_API_KEY environment variable.';