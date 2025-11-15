# Error Codes and Messages

This page provides a comprehensive reference for all error codes and messages that you might encounter while using the pg_ai_query extension.

## Function-Specific Errors

### generate_query Errors

| Error Message | Cause | Solution |
|---------------|-------|----------|
| `"API key required. Pass as parameter or configure ~/.pg_ai.config"` | No API key provided and none configured | Add API key to config file or pass as parameter |
| `"No API key available for [provider] provider"` | API key missing for specific provider | Configure API key for the requested provider |
| `"AI API error: [details]"` | AI service returned an error | Check API key validity and service status |
| `"Natural language query cannot be empty"` | Empty input provided | Provide a non-empty query description |
| `"Query generation failed: [details]"` | AI failed to generate query | Check your description clarity and try again |

### explain_query Errors

| Error Message | Cause | Solution |
|---------------|-------|----------|
| `"Query text cannot be empty"` | Empty query provided | Provide a valid SQL query |
| `"Failed to prepare EXPLAIN query: syntax error"` | Invalid SQL syntax | Fix SQL syntax errors |
| `"Failed to execute EXPLAIN query"` | Query execution failed | Check query validity and permissions |
| `"Failed to connect to SPI"` | Database connection issue | Check database connectivity |
| `"No output from EXPLAIN query"` | EXPLAIN returned no results | Verify query syntax and structure |

### get_database_tables Errors

| Error Message | Cause | Solution |
|---------------|-------|----------|
| `"Failed to connect to SPI"` | Database connection issue | Check database connectivity |
| `"Failed to execute query"` | Database query failed | Check permissions and database status |
| `"No tables found"` | No user tables in database | Create tables or check schema permissions |

### get_table_details Errors

| Error Message | Cause | Solution |
|---------------|-------|----------|
| `"Table '[table_name]' does not exist"` | Specified table not found | Check table name and schema |
| `"Failed to get table details: [details]"` | Error retrieving table info | Check permissions and table existence |

## Debugging Commands

```sql
-- Check extension status
SELECT extname, extversion FROM pg_extension WHERE extname = 'pg_ai_query';

-- Test basic functionality
SELECT generate_query('SELECT 1');

-- Check function permissions
\df generate_query
```

## See Also

- [Troubleshooting Guide](./troubleshooting.md)
- [Function Reference](./function-reference.md)