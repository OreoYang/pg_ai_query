# get_table_details Function

The `get_table_details` function returns detailed information about a specific table including columns, constraints, foreign keys, and indexes.

## Function Signature

```sql
get_table_details(
    table_name text,
    schema_name text DEFAULT 'public'
) RETURNS text
```

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `table_name` | `text` | *required* | Name of the table to analyze |
| `schema_name` | `text` | `'public'` | Schema containing the table |

## Basic Usage

```sql
-- Basic usage
SELECT get_table_details('users');

-- Specific schema
SELECT get_table_details('orders', 'sales');

-- Pretty print
SELECT jsonb_pretty(get_table_details('users')::jsonb);
```

## Output Format

The function returns detailed JSON information about the table:

```json
{
  "table_name": "users",
  "schema_name": "public",
  "columns": [
    {
      "column_name": "id",
      "data_type": "integer",
      "is_nullable": false,
      "column_default": "nextval('users_id_seq'::regclass)",
      "is_primary_key": true,
      "is_foreign_key": false
    },
    {
      "column_name": "email",
      "data_type": "character varying",
      "is_nullable": false,
      "column_default": null,
      "is_primary_key": false,
      "is_foreign_key": false
    }
  ],
  "indexes": [
    "users_pkey",
    "users_email_key"
  ]
}
```

## Extracting Information

### Get Column Information

```sql
SELECT
    column_name,
    data_type,
    is_nullable,
    is_primary_key
FROM jsonb_to_recordset(
    (get_table_details('users')::jsonb)->'columns'
) AS x(
    column_name text,
    data_type text,
    is_nullable boolean,
    is_primary_key boolean
);
```

### Find Primary Key Columns

```sql
SELECT
    column_name
FROM jsonb_to_recordset(
    (get_table_details('users')::jsonb)->'columns'
) AS x(column_name text, is_primary_key boolean)
WHERE is_primary_key = true;
```

### List All Indexes

```sql
SELECT
    jsonb_array_elements_text(
        (get_table_details('orders')::jsonb)->'indexes'
    ) as index_name;
```

## Use Cases

- **Schema Documentation**: Generate comprehensive table documentation
- **Database Exploration**: Understand table structure before writing queries
- **Data Migration Planning**: Analyze table structure for migration scripts
- **Query Optimization**: Understand available indexes and constraints
- **Integration Planning**: Map foreign key relationships

## Common Patterns

### Analyzing All Tables

```sql
-- Get details for all tables in database
WITH all_tables AS (
    SELECT
        jsonb_array_elements(get_database_tables()::jsonb)->>'table_name' as table_name
)
SELECT
    table_name,
    get_table_details(table_name) as details
FROM all_tables;
```

### Finding Foreign Key Relationships

```sql
SELECT
    column_name,
    foreign_table,
    foreign_column
FROM jsonb_to_recordset(
    (get_table_details('orders')::jsonb)->'columns'
) AS x(
    column_name text,
    is_foreign_key boolean,
    foreign_table text,
    foreign_column text
)
WHERE is_foreign_key = true;
```

## Integration with Other Functions

### With generate_query

```sql
-- First understand the table structure
SELECT get_table_details('users');

-- Then generate queries based on that knowledge
SELECT generate_query('show users with their profile information');
```

### With explain_query

```sql
-- Analyze table structure
SELECT get_table_details('orders');

-- Generate and analyze a query
SELECT explain_query('SELECT * FROM orders WHERE user_id = 123');
```

## Error Handling

Common errors and solutions:

| Error | Cause | Solution |
|-------|-------|----------|
| `"Table 'tablename' does not exist"` | Table not found | Check table name and schema |
| `"Access denied to table"` | Insufficient permissions | Grant SELECT permission on table |
| `"Schema 'schemaname' does not exist"` | Schema not found | Verify schema name |

## See Also

- [get_database_tables Function](./database-tables.md) - List all database tables
- [generate_query Function](./generate-query.md) - Generate queries from natural language
- [Error Codes](./error-codes.md) - Troubleshooting guide