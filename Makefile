EXTENSION = pg_ai_query
DATA = sql/pg_ai_query--1.0.sql
MODULES = pg_ai_query

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

#Build Rules
BUILD_DIR = build
all: pg_ai_query.so

pg_ai_query.so: $(BUILD_DIR)/CMakeCache.txt
	$(MAKE) -C $(BUILD_DIR)
	cp $(BUILD_DIR)/pg_ai_query.so .

$(BUILD_DIR)/CMakeCache.txt:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DCMAKE_INSTALL_PREFIX=$(shell $(PG_CONFIG) --pkglibdir)

clean:
	rm -rf $(BUILD_DIR) install *.so

.PHONY: all clean

# Formatting
SRC_FILES = $(shell find . -type f \
	\( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \
	   -o -name "*.h" -o -name "*.hpp" \) \
	-not -path "./$(BUILD_DIR)/*" \
	-not -path "./third_party/*")

# Run clang-format in-place
format:
	@echo "Formatting $(words $(SRC_FILES)) file(s)..."
	@clang-format -i -style=file $(SRC_FILES)

# Dry-run: show a diff of what *would* be changed
format-check:
	@echo "Checking formatting..."
	@clang-format -style=file --dry-run --Werror $(SRC_FILES) \
		|| (echo "ERROR: Code is not formatted. Run 'make format'." && exit 1)

.PHONY: format format-check