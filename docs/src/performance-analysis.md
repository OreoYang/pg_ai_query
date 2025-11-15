# Query Performance Analysis

Learn how to use the `explain_query` function to analyze and optimize your PostgreSQL queries with AI-powered insights.

## Introduction

Query performance analysis is crucial for maintaining fast, efficient databases. The `explain_query` function combines PostgreSQL's EXPLAIN ANALYZE with advanced AI analysis to provide actionable insights for query optimization.

## Getting Started

### Basic Analysis

Start with a simple query to understand the output format:

```sql
SELECT explain_query('SELECT id, email FROM users WHERE active = true LIMIT 100');
```

This will give you:
- How PostgreSQL executes the query
- Whether indexes are being used
- Performance metrics and timing
- AI-generated optimization suggestions

### Analyzing Slow Queries

If you have a query that's running slowly:

```sql
SELECT explain_query('
    SELECT u.username, COUNT(o.id) as total_orders
    FROM users u
    LEFT JOIN orders o ON u.id = o.user_id
    WHERE u.created_at > ''2024-01-01''
    GROUP BY u.username
    ORDER BY total_orders DESC
');
```

Look for:
- Sequential scans on large tables
- Missing indexes
- Inefficient join operations
- Sort operations without indexes

## Common Performance Patterns

### Index Usage Analysis

#### Checking if Indexes are Used

```sql
-- This should use an index if one exists on user_id
SELECT explain_query('SELECT * FROM orders WHERE user_id = 123');

-- This might not use an index efficiently
SELECT explain_query('SELECT * FROM orders WHERE UPPER(status) = ''PENDING''');
```

#### Composite Index Analysis

```sql
-- Check if a composite index would help
SELECT explain_query('
    SELECT * FROM orders
    WHERE status = ''completed''
    AND created_at > NOW() - INTERVAL ''30 days''
    ORDER BY created_at DESC
');
```

Look for recommendations about creating indexes like:
```sql
CREATE INDEX idx_orders_status_created ON orders(status, created_at);
```

### Join Performance

#### Nested Loop vs Hash Join

```sql
-- Small result set - might use nested loop
SELECT explain_query('
    SELECT u.email, o.total
    FROM users u
    JOIN orders o ON u.id = o.user_id
    WHERE u.id = 123
');

-- Large result set - should use hash or merge join
SELECT explain_query('
    SELECT u.username, COUNT(o.id)
    FROM users u
    JOIN orders o ON u.id = o.user_id
    GROUP BY u.username
');
```

#### Optimizing Join Order

```sql
-- Multiple joins - check join order efficiency
SELECT explain_query('
    SELECT u.username, p.name as product, oi.quantity
    FROM users u
    JOIN orders o ON u.id = o.user_id
    JOIN order_items oi ON o.id = oi.order_id
    JOIN products p ON oi.product_id = p.id
    WHERE o.created_at > NOW() - INTERVAL ''7 days''
');
```

### Aggregation Performance

#### GROUP BY Analysis

```sql
-- Check if grouping is efficient
SELECT explain_query('
    SELECT DATE(created_at), COUNT(*)
    FROM orders
    WHERE created_at >= ''2024-01-01''
    GROUP BY DATE(created_at)
    ORDER BY DATE(created_at)
');
```

#### Window Functions

```sql
-- Analyze window function performance
SELECT explain_query('
    SELECT
        user_id,
        created_at,
        ROW_NUMBER() OVER (PARTITION BY user_id ORDER BY created_at) as order_number
    FROM orders
    WHERE created_at > NOW() - INTERVAL ''1 month''
');
```

## Optimization Workflows

### Before and After Comparison

#### 1. Analyze Original Query
```sql
SELECT explain_query('
    SELECT * FROM orders o, users u
    WHERE o.user_id = u.id AND o.total > 100
');
```

#### 2. Apply Optimizations
Based on the AI recommendations, rewrite the query:

```sql
SELECT explain_query('
    SELECT o.id, o.total, u.username
    FROM orders o
    INNER JOIN users u ON o.user_id = u.id
    WHERE o.total > 100
');
```

#### 3. Create Recommended Indexes
```sql
-- Example recommendation from AI analysis
CREATE INDEX idx_orders_total ON orders(total) WHERE total > 0;
```

#### 4. Re-analyze
```sql
SELECT explain_query('
    SELECT o.id, o.total, u.username
    FROM orders o
    INNER JOIN users u ON o.user_id = u.id
    WHERE o.total > 100
');
```

### Systematic Query Review

Create a systematic approach to review your application's queries:

```sql
-- Create a function to analyze multiple queries
CREATE OR REPLACE FUNCTION analyze_app_queries()
RETURNS TABLE(query_name text, analysis text) AS $$
BEGIN
    RETURN QUERY
    SELECT 'user_dashboard'::text,
           explain_query('SELECT * FROM users WHERE last_login > NOW() - INTERVAL ''30 days''')
    UNION ALL
    SELECT 'order_summary'::text,
           explain_query('SELECT COUNT(*), SUM(total) FROM orders WHERE created_at >= CURRENT_DATE')
    UNION ALL
    SELECT 'top_products'::text,
           explain_query('
               SELECT p.name, SUM(oi.quantity) as sold
               FROM products p
               JOIN order_items oi ON p.id = oi.product_id
               JOIN orders o ON oi.order_id = o.id
               WHERE o.created_at >= CURRENT_DATE - INTERVAL ''7 days''
               GROUP BY p.id, p.name
               ORDER BY sold DESC
               LIMIT 10
           ');
END;
$$ LANGUAGE plpgsql;

-- Run the analysis
SELECT * FROM analyze_app_queries();
```

## Performance Monitoring

### Regular Health Checks

Set up regular performance monitoring:

```sql
-- Monitor daily active users query
SELECT explain_query('
    SELECT COUNT(DISTINCT user_id)
    FROM user_sessions
    WHERE DATE(created_at) = CURRENT_DATE
');

-- Monitor order processing performance
SELECT explain_query('
    SELECT AVG(processing_time_seconds)
    FROM orders
    WHERE status = ''completed''
    AND created_at >= CURRENT_DATE
');
```

### Automated Analysis

Create views for continuous monitoring:

```sql
CREATE VIEW query_performance_monitor AS
SELECT
    'recent_orders' as query_type,
    explain_query('SELECT * FROM orders WHERE created_at > NOW() - INTERVAL ''1 hour''') as analysis
UNION ALL
SELECT
    'active_sessions',
    explain_query('SELECT COUNT(*) FROM user_sessions WHERE last_activity > NOW() - INTERVAL ''15 minutes''');
```

## Advanced Analysis Techniques

### Subquery vs JOIN Performance

```sql
-- Analyze EXISTS subquery
SELECT explain_query('
    SELECT u.username
    FROM users u
    WHERE EXISTS (
        SELECT 1 FROM orders o
        WHERE o.user_id = u.id
        AND o.total > 1000
    )
');

-- Compare with JOIN approach
SELECT explain_query('
    SELECT DISTINCT u.username
    FROM users u
    JOIN orders o ON u.id = o.user_id
    WHERE o.total > 1000
');
```

### CTE vs Subquery Analysis

```sql
-- Common Table Expression approach
SELECT explain_query('
    WITH high_value_customers AS (
        SELECT user_id, SUM(total) as total_spent
        FROM orders
        WHERE created_at > NOW() - INTERVAL ''1 year''
        GROUP BY user_id
        HAVING SUM(total) > 5000
    )
    SELECT u.username, hvc.total_spent
    FROM users u
    JOIN high_value_customers hvc ON u.id = hvc.user_id
');

-- Subquery approach
SELECT explain_query('
    SELECT u.username, sub.total_spent
    FROM users u
    JOIN (
        SELECT user_id, SUM(total) as total_spent
        FROM orders
        WHERE created_at > NOW() - INTERVAL ''1 year''
        GROUP BY user_id
        HAVING SUM(total) > 5000
    ) sub ON u.id = sub.user_id
');
```

### Partition Analysis

For partitioned tables:

```sql
-- Check if partition pruning is working
SELECT explain_query('
    SELECT COUNT(*)
    FROM orders_2024
    WHERE created_at BETWEEN ''2024-06-01'' AND ''2024-06-30''
');
```

## Interpreting AI Recommendations

### Understanding Cost Estimates

- **Total Cost**: PostgreSQL's relative measure of query expense
- **Startup Cost**: Cost before returning first row
- **Execution Time**: Actual time taken (from ANALYZE)
- **Rows**: Estimated vs actual rows processed

### Index Recommendations

The AI will suggest:
- **Simple indexes**: `CREATE INDEX idx_table_column ON table(column);`
- **Composite indexes**: `CREATE INDEX idx_table_multi ON table(col1, col2);`
- **Partial indexes**: `CREATE INDEX idx_table_partial ON table(column) WHERE condition;`
- **Expression indexes**: `CREATE INDEX idx_table_expr ON table(LOWER(column));`

### Query Rewriting Suggestions

Common recommendations include:
- Rewriting `WHERE` clauses to be more selective
- Changing join order or join types
- Using `LIMIT` more effectively
- Replacing correlated subqueries with joins

## Troubleshooting Common Issues

### No Index Usage

If indexes aren't being used:

```sql
-- Check if the query can use the index
SELECT explain_query('SELECT * FROM users WHERE email = ''user@example.com''');

-- Vs a query that can't use the index efficiently
SELECT explain_query('SELECT * FROM users WHERE email LIKE ''%@example.com''');
```

### Slow Aggregations

For slow GROUP BY queries:

```sql
-- Analyze aggregation performance
SELECT explain_query('
    SELECT product_category, COUNT(*), AVG(price)
    FROM products
    GROUP BY product_category
');
```

### Large Result Sets

When dealing with large results:

```sql
-- Use LIMIT to test query efficiency
SELECT explain_query('
    SELECT * FROM orders
    ORDER BY created_at DESC
    LIMIT 100
');
```

## Best Practices

1. **Start Small**: Begin with `LIMIT` to understand query patterns
2. **Focus on Bottlenecks**: Look for the most expensive operations first
3. **Test Incrementally**: Make one optimization at a time
4. **Validate Results**: Ensure optimizations actually improve performance
5. **Monitor Regularly**: Set up regular analysis of critical queries
6. **Document Changes**: Keep track of optimizations and their impact

## Integration with Development Workflow

### Code Review Process

```sql
-- Add performance analysis to your development process
CREATE OR REPLACE FUNCTION review_new_query(query_text text)
RETURNS text AS $$
BEGIN
    RETURN explain_query(query_text);
END;
$$ LANGUAGE plpgsql;

-- Use in code reviews
SELECT review_new_query('SELECT * FROM new_feature_table WHERE complex_condition = true');
```

### CI/CD Integration

Create performance tests:

```sql
-- Performance regression test
DO $$
DECLARE
    analysis text;
BEGIN
    SELECT explain_query('SELECT * FROM critical_table WHERE important_column = 123') INTO analysis;

    -- Log or validate the analysis
    IF analysis LIKE '%Sequential Scan%' THEN
        RAISE WARNING 'Performance regression detected: Sequential scan in critical query';
    END IF;
END;
$$;
```

## Next Steps

- [Function Reference](./function-reference.md) - Complete API documentation
- [Best Practices](./best-practices.md) - Advanced optimization techniques
- [Integration Patterns](./integration.md) - Using with other tools
- [Troubleshooting](./troubleshooting.md) - Common issues and solutions