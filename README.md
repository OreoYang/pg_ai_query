# PostgreSQL AI Query Generator

A clean, efficient PostgreSQL extension that generates SQL queries from natural language using OpenAI's GPT models.

## Features

- **Simple API**: One function `pg_generate_query()` for generating SQL
- **Safety First**: Only generates SELECT queries, validates for dangerous operations
- **Environment Driven**: Uses `OPENAI_API_KEY` environment variable
- **Clean Implementation**: Direct AI SDK usage without over-engineering

## Installation

1. **Prerequisites**:
   - PostgreSQL development headers
   - CMake 3.16+
   - OpenAI API key

2. **Build and Install**:
   ```bash
   git clone --recurse-submodules <repo-url>
   cd pg_ai_queri
   export OPENAI_API_KEY=your_key_here
   make install
   ```

3. **Enable Extension**:
   ```sql
   CREATE EXTENSION pg_ai_query;
   ```

## Usage

```sql
-- Basic query generation
SELECT pg_generate_query('Show me all users created in the last week');

-- With table context
SELECT pg_generate_query('Count orders by status', 'orders');

-- With schema information
SELECT pg_generate_query(
    'Find top 5 customers by order value',
    'customers',
    'Tables: customers(id, name, email), orders(id, customer_id, total, created_at)'
);
```

## Architecture

### Clean Implementation (125 lines vs 864 lines original)

- **`QueryGenerator`**: Single static class with core logic
- **Direct AI SDK**: Uses `ai::openai::create_client()` without wrappers
- **Environment Config**: No complex configuration management
- **Built-in Validation**: Essential safety checks only

### Files

```
src/
├── pg_ai_query.cpp              # PostgreSQL C interface
├── core/
│   └── query_generator.cpp      # Core AI query generation
└── include/
    └── query_generator.hpp      # Clean header interface
```

### Key Simplifications Made

1. **Removed ConfigManager** → Uses `OPENAI_API_KEY` directly
2. **Removed PIMPL Pattern** → Direct implementation
3. **Removed Custom Errors** → Uses AI SDK error handling
4. **Removed Complex Validation** → Essential safety checks only
5. **Uses AI SDK Constants** → `ai::openai::models::kGpt4o` instead of strings

## Safety

- Only generates SELECT statements
- Validates against dangerous SQL operations
- Blocks INSERT, UPDATE, DELETE, DROP, CREATE, etc.
- Uses OpenAI with conservative temperature (0.1)

## Development

The codebase follows the ai-sdk-cpp examples pattern for clean, maintainable code:

```cpp
auto client = ai::openai::create_client();  // From environment
ai::GenerateOptions options;
options.model = ai::openai::models::kGpt4o;
options.prompt = prompt;
auto result = client.generate_text(options);
```

## License

See LICENSE file.
