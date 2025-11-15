# get_database_tables Function

The `get_database_tables` function returns metadata about all user tables in the database, providing a comprehensive overview of your database schema.

## Function Signature

```sql
get_database_tables() RETURNS text
```

## Parameters

This function takes no parameters.

## Basic Usage

```sql
-- Get all database tables
SELECT get_database_tables();

-- Pretty print with formatting
SELECT jsonb_pretty(get_database_tables()::jsonb);
```

## Output Format

The function returns a JSON array containing metadata for each table:

```json
[
  {
    "table_name": "users",
    "schema_name": "public",
    "table_type": "BASE TABLE",
    "estimated_rows": 1500
  },
  {
    "table_name": "orders",
    "schema_name": "public",
    "table_type": "BASE TABLE",
    "estimated_rows": 5000
  }
]
```

## Extracting Information

### Get Table Names Only

```sql
SELECT
    jsonb_array_elements(get_database_tables()::jsonb)->>'table_name' as table_name
FROM (SELECT get_database_tables()) t;
```

### Get Tables with Row Counts

```sql
SELECT
    table_name,
    estimated_rows
FROM jsonb_to_recordset(get_database_tables()::jsonb)
AS x(table_name text, estimated_rows int)
ORDER BY estimated_rows DESC;
```

## Use Cases

- Database exploration and documentation
- Schema discovery for applications
- Inventory of available tables
- Integration with generate_query function

## See Also

- [get_table_details Function](./table-details.md) - Get detailed table information
- [generate_query Function](./generate-query.md) - Generate queries from natural language