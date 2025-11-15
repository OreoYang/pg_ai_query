# Integration Patterns

This guide covers common patterns for integrating pg_ai_query with applications, development workflows, and data analysis tools.

## Application Integration

### Python Integration

```python
import psycopg2
import json

class NaturalLanguageDB:
    def __init__(self, connection_string):
        self.conn = psycopg2.connect(connection_string)

    def query(self, description, execute=False):
        """Generate and optionally execute a query from natural language"""
        cur = self.conn.cursor()

        # Generate the query
        cur.execute("SELECT generate_query(%s)", (description,))
        query = cur.fetchone()[0]

        if execute:
            # Execute the generated query
            cur.execute(query)
            results = cur.fetchall()
            return query, results
        else:
            return query, None

    def analyze_performance(self, query):
        """Analyze query performance"""
        cur = self.conn.cursor()
        cur.execute("SELECT explain_query(%s)", (query,))
        return cur.fetchone()[0]

# Usage example
db = NaturalLanguageDB("postgresql://user:pass@localhost/dbname")

# Generate a query
query, _ = db.query("show top customers by revenue")
print(f"Generated: {query}")

# Execute and get results
query, results = db.query("count orders by status", execute=True)
print(f"Results: {results}")

# Analyze performance
analysis = db.analyze_performance(query)
print(f"Performance: {analysis}")
```

### Node.js Integration

```javascript
const { Pool } = require('pg');

class AIQueryClient {
    constructor(config) {
        this.pool = new Pool(config);
    }

    async generateQuery(description, apiKey = null, provider = 'auto') {
        const client = await this.pool.connect();
        try {
            const result = await client.query(
                'SELECT generate_query($1, $2, $3)',
                [description, apiKey, provider]
            );
            return result.rows[0].generate_query;
        } finally {
            client.release();
        }
    }

    async executeNaturalQuery(description) {
        const query = await this.generateQuery(description);
        const client = await this.pool.connect();
        try {
            const result = await client.query(query);
            return {
                query: query,
                results: result.rows,
                rowCount: result.rowCount
            };
        } finally {
            client.release();
        }
    }

    async analyzeQuery(query) {
        const client = await this.pool.connect();
        try {
            const result = await client.query(
                'SELECT explain_query($1)',
                [query]
            );
            return result.rows[0].explain_query;
        } finally {
            client.release();
        }
    }
}

// Usage
const aiClient = new AIQueryClient({
    user: 'postgres',
    host: 'localhost',
    database: 'myapp',
    password: 'password',
    port: 5432,
});

// Generate and execute
aiClient.executeNaturalQuery('show recent orders')
    .then(result => {
        console.log('Query:', result.query);
        console.log('Results:', result.results);
    });
```

## Development Workflow Integration

### Code Review Integration

Create a function to validate new queries during code review:

```sql
-- Create a query validation function
CREATE OR REPLACE FUNCTION validate_query_performance(query_text text)
RETURNS TABLE(
    performance_score int,
    issues text[],
    recommendations text[]
) AS $$
DECLARE
    analysis text;
    score int := 100;
    issues_array text[] := '{}';
    recommendations_array text[] := '{}';
BEGIN
    -- Get AI analysis
    SELECT explain_query(query_text) INTO analysis;

    -- Simple scoring based on analysis content
    IF analysis ILIKE '%sequential scan%' THEN
        score := score - 30;
        issues_array := issues_array || 'Sequential scan detected';
        recommendations_array := recommendations_array || 'Consider adding indexes';
    END IF;

    IF analysis ILIKE '%high cost%' OR analysis ILIKE '%expensive%' THEN
        score := score - 20;
        issues_array := issues_array || 'High execution cost';
    END IF;

    RETURN QUERY SELECT score, issues_array, recommendations_array;
END;
$$ LANGUAGE plpgsql;

-- Usage in CI/CD
SELECT * FROM validate_query_performance('
    SELECT * FROM orders o
    JOIN users u ON o.user_id = u.id
    WHERE o.created_at > NOW() - INTERVAL ''30 days''
');
```

### Git Hooks Integration

```bash
#!/bin/bash
# .git/hooks/pre-commit

# Extract SQL queries from staged files
git diff --cached --name-only | grep '\.sql$' | while read file; do
    echo "Analyzing SQL file: $file"

    # Extract queries and analyze with pg_ai_query
    psql -d mydb -c "
        SELECT explain_query(\$\$$(cat $file)\$\$)
    " | grep -i 'warning\|issue\|slow'

    if [ $? -eq 0 ]; then
        echo "Performance issues detected in $file"
        exit 1
    fi
done
```

## Business Intelligence Integration

### Jupyter Notebook Integration

```python
# Install: pip install psycopg2-binary pandas plotly

import pandas as pd
import psycopg2
import plotly.express as px

def natural_query_to_dataframe(connection_string, description):
    """Convert natural language to SQL and return pandas DataFrame"""
    with psycopg2.connect(connection_string) as conn:
        # Generate query
        query_df = pd.read_sql("SELECT generate_query(%s) as query", conn, params=[description])
        query = query_df.iloc[0]['query']

        # Execute query
        result_df = pd.read_sql(query, conn)

        print(f"Generated Query: {query}")
        return result_df

# Usage in Jupyter
df = natural_query_to_dataframe(
    "postgresql://user:pass@localhost/db",
    "monthly revenue trend for the last year"
)

# Create visualization
fig = px.line(df, x='month', y='revenue', title='Monthly Revenue Trend')
fig.show()
```

### Grafana Integration

Create a PostgreSQL data source query that uses pg_ai_query:

```sql
-- Grafana query template
SELECT
    time_column,
    value_column
FROM (
    ${__query(SELECT generate_query('${natural_description:text}'))}
) generated_query
WHERE $__timeFilter(time_column)
```

## Data Analysis Workflows

### Automated Reporting

```python
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
import psycopg2
import pandas as pd
from datetime import datetime, timedelta

class AutomatedReporter:
    def __init__(self, db_config, email_config):
        self.db_config = db_config
        self.email_config = email_config

    def generate_report(self, queries_descriptions):
        """Generate automated reports from natural language queries"""
        report_data = {}

        with psycopg2.connect(**self.db_config) as conn:
            for name, description in queries_descriptions.items():
                # Generate and execute query
                df = pd.read_sql(
                    "SELECT generate_query(%s) as query",
                    conn, params=[description]
                )
                query = df.iloc[0]['query']

                # Execute the generated query
                result_df = pd.read_sql(query, conn)
                report_data[name] = result_df

                # Analyze performance
                analysis_df = pd.read_sql(
                    "SELECT explain_query(%s) as analysis",
                    conn, params=[query]
                )
                print(f"Performance analysis for {name}:")
                print(analysis_df.iloc[0]['analysis'])

        return report_data

    def send_report(self, report_data):
        """Send the generated report via email"""
        # Create email content from report data
        # Implementation depends on your email service
        pass

# Usage
reporter = AutomatedReporter(
    db_config={'host': 'localhost', 'database': 'mydb', ...},
    email_config={'smtp_server': 'smtp.gmail.com', ...}
)

daily_queries = {
    'user_signups': 'daily user registrations for the last 7 days',
    'revenue': 'daily revenue for the last 7 days',
    'top_products': 'top 10 products by sales today'
}

report = reporter.generate_report(daily_queries)
```

### Query Performance Monitoring

```sql
-- Create a monitoring system
CREATE TABLE query_performance_log (
    id SERIAL PRIMARY KEY,
    query_description text,
    generated_query text,
    execution_time interval,
    analysis_summary text,
    created_at timestamp DEFAULT NOW()
);

-- Function to log and analyze queries
CREATE OR REPLACE FUNCTION monitor_query_performance(description text)
RETURNS void AS $$
DECLARE
    query text;
    start_time timestamp;
    end_time timestamp;
    exec_time interval;
    analysis text;
BEGIN
    -- Generate query
    SELECT generate_query(description) INTO query;

    -- Time the execution
    start_time := clock_timestamp();
    EXECUTE query;
    end_time := clock_timestamp();
    exec_time := end_time - start_time;

    -- Get performance analysis
    SELECT explain_query(query) INTO analysis;

    -- Log the results
    INSERT INTO query_performance_log
    (query_description, generated_query, execution_time, analysis_summary)
    VALUES (description, query, exec_time, analysis);

    -- Alert on slow queries
    IF exec_time > interval '5 seconds' THEN
        RAISE WARNING 'Slow query detected: % took %', description, exec_time;
    END IF;
END;
$$ LANGUAGE plpgsql;

-- Usage
SELECT monitor_query_performance('daily active users count');
```

## API Integration Patterns

### REST API Wrapper

```python
from flask import Flask, request, jsonify
import psycopg2
import os

app = Flask(__name__)

def get_db_connection():
    return psycopg2.connect(
        host=os.getenv('DB_HOST'),
        database=os.getenv('DB_NAME'),
        user=os.getenv('DB_USER'),
        password=os.getenv('DB_PASSWORD')
    )

@app.route('/api/query', methods=['POST'])
def natural_language_query():
    """Generate and optionally execute queries from natural language"""
    data = request.json
    description = data.get('description')
    execute = data.get('execute', False)
    api_key = data.get('api_key')
    provider = data.get('provider', 'auto')

    if not description:
        return jsonify({'error': 'Description required'}), 400

    try:
        conn = get_db_connection()
        cur = conn.cursor()

        # Generate query
        cur.execute(
            "SELECT generate_query(%s, %s, %s)",
            (description, api_key, provider)
        )
        query = cur.fetchone()[0]

        result = {'query': query}

        if execute:
            # Execute the generated query
            cur.execute(query)
            results = cur.fetchall()
            columns = [desc[0] for desc in cur.description]

            result['results'] = [dict(zip(columns, row)) for row in results]
            result['row_count'] = len(results)

        conn.close()
        return jsonify(result)

    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/explain', methods=['POST'])
def explain_query():
    """Analyze query performance"""
    data = request.json
    query = data.get('query')

    if not query:
        return jsonify({'error': 'Query required'}), 400

    try:
        conn = get_db_connection()
        cur = conn.cursor()

        cur.execute("SELECT explain_query(%s)", (query,))
        analysis = cur.fetchone()[0]

        conn.close()
        return jsonify({'analysis': analysis})

    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True)
```

## Testing Integration

### Unit Testing Queries

```python
import unittest
import psycopg2

class TestQueryGeneration(unittest.TestCase):
    def setUp(self):
        self.conn = psycopg2.connect("postgresql://test_user:test_pass@localhost/test_db")

    def test_basic_query_generation(self):
        """Test basic query generation functionality"""
        cur = self.conn.cursor()

        # Test simple query
        cur.execute("SELECT generate_query('show all users')")
        query = cur.fetchone()[0]

        self.assertIn('SELECT', query.upper())
        self.assertIn('users', query.lower())

    def test_query_performance(self):
        """Test that generated queries perform reasonably well"""
        cur = self.conn.cursor()

        # Generate a query
        cur.execute("SELECT generate_query('count active users')")
        query = cur.fetchone()[0]

        # Analyze performance
        cur.execute("SELECT explain_query(%s)", (query,))
        analysis = cur.fetchone()[0]

        # Check for performance issues
        self.assertNotIn('Sequential Scan', analysis)

    def tearDown(self):
        self.conn.close()
```

## Production Deployment Patterns

### Connection Pooling

```python
from psycopg2 import pool
import threading

class AIQueryPool:
    def __init__(self, minconn, maxconn, **kwargs):
        self.pool = pool.ThreadedConnectionPool(
            minconn, maxconn, **kwargs
        )
        self.lock = threading.Lock()

    def execute_natural_query(self, description):
        conn = self.pool.getconn()
        try:
            cur = conn.cursor()
            cur.execute("SELECT generate_query(%s)", (description,))
            query = cur.fetchone()[0]

            cur.execute(query)
            results = cur.fetchall()
            return query, results
        finally:
            self.pool.putconn(conn)

# Usage
query_pool = AIQueryPool(
    minconn=5, maxconn=20,
    host='localhost', database='prod_db',
    user='app_user', password='secure_password'
)
```

### Error Handling and Logging

```python
import logging
from functools import wraps

def handle_ai_query_errors(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except psycopg2.Error as e:
            logging.error(f"Database error in {func.__name__}: {e}")
            raise
        except Exception as e:
            logging.error(f"Unexpected error in {func.__name__}: {e}")
            raise
    return wrapper

@handle_ai_query_errors
def safe_natural_query(connection, description):
    """Safely execute natural language queries with error handling"""
    cur = connection.cursor()
    cur.execute("SELECT generate_query(%s)", (description,))
    return cur.fetchone()[0]
```

## See Also

- [Function Reference](./function-reference.md) - Complete API documentation
- [Best Practices](./best-practices.md) - Performance and security guidelines
- [Examples](./examples.md) - Additional usage examples
- [Error Codes](./error-codes.md) - Troubleshooting guide