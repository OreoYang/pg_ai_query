cmake_minimum_required(VERSION 3.16)
project(pg_ai_query)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find PostgreSQL
find_program(PG_CONFIG pg_config REQUIRED)
execute_process(COMMAND ${PG_CONFIG} --includedir-server OUTPUT_VARIABLE PG_INCLUDE_SERVER OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PG_CONFIG} --includedir OUTPUT_VARIABLE PG_INCLUDE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PG_CONFIG} --pkglibdir OUTPUT_VARIABLE PG_PKGLIB OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PG_CONFIG} --libdir OUTPUT_VARIABLE PG_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PG_CONFIG} --bindir OUTPUT_VARIABLE PG_BINDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

# Include PostgreSQL headers
include_directories(${PG_INCLUDE_SERVER})
include_directories(${PG_INCLUDE})

# Configure ai-sdk-cpp options
set(AI_SDK_BUILD_EXAMPLES OFF CACHE BOOL "")
set(AI_SDK_BUILD_TESTS OFF CACHE BOOL "")

# Add the ai-sdk-cpp submodule
add_subdirectory(third_party/ai-sdk-cpp)

# Include our source directories
include_directories(src)

# Source files for the clean implementation
set(SOURCES
    src/pg_ai_query.cpp
    src/core/query_generator.cpp
    src/core/logger.cpp
    src/utils.cpp
    src/prompts.cpp
    src/config.cpp
)

# Create module library for PostgreSQL extension
if(APPLE)
    add_library(pg_ai_query MODULE ${SOURCES})
    set_target_properties(pg_ai_query PROPERTIES
        PREFIX ""
        SUFFIX ".so"
        LINK_FLAGS "-bundle_loader ${PG_BINDIR}/postgres"
    )
else()
    add_library(pg_ai_query SHARED ${SOURCES})
    set_target_properties(pg_ai_query PROPERTIES
        PREFIX ""
        SUFFIX ".so"
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Link with ai-sdk-cpp
target_link_libraries(pg_ai_query PRIVATE
    ai-sdk-cpp
)

# Include directories
target_include_directories(pg_ai_query PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ai-sdk-cpp/include
)

# Add compile definitions
target_compile_definitions(pg_ai_query PRIVATE
    USE_POSTGRESQL_ELOG
)

# Install target
install(TARGETS pg_ai_query
    LIBRARY DESTINATION ${PG_PKGLIB}
)

# Install SQL files
install(FILES sql/pg_ai_query--1.0.sql
    DESTINATION ${PG_PKGLIB}/../extension/
)

install(FILES pg_ai_query.control
    DESTINATION ${PG_PKGLIB}/../extension/
)

# Install prompts directory
install(DIRECTORY prompts/
    DESTINATION ${PG_PKGLIB}/../extension/prompts/
    FILES_MATCHING PATTERN "*.txt"
)

# Optional: Build logger test (not built by default)
# Uncomment to build: cmake .. -DBUILD_LOGGER_TEST=ON
option(BUILD_LOGGER_TEST "Build logger test executable" OFF)
if(BUILD_LOGGER_TEST)
    add_executable(test_logger
        src/test_logger.cpp
        src/core/logger.cpp
    )
    target_include_directories(test_logger PRIVATE src)
endif()