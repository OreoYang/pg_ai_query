EXTENSION = pg_ai_query
DATA = sql/pg_ai_query--1.0.sql
MODULES = pg_ai_query

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

#Build Rules
BUILD_DIR = build

# Determine extension suffix based on PostgreSQL version on macOS
ifeq ($(shell uname),Darwin)
    PG_VERSION := $(shell $(PG_CONFIG) --version | sed 's/PostgreSQL \([0-9]*\).*/\1/')
    ifeq ($(shell test $(PG_VERSION) -ge 16; echo $$?),0)
        EXT_SUFFIX := .dylib
    else
        EXT_SUFFIX := .so
    endif
else
    EXT_SUFFIX := .so
endif

TARGET_LIB := pg_ai_query$(EXT_SUFFIX)

all: $(TARGET_LIB)

$(TARGET_LIB): $(BUILD_DIR)/CMakeCache.txt
	$(MAKE) -C $(BUILD_DIR)
	cp $(BUILD_DIR)/$(TARGET_LIB) .

$(BUILD_DIR)/CMakeCache.txt:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DCMAKE_INSTALL_PREFIX=$(shell $(PG_CONFIG) --pkglibdir)

clean:
	rm -rf $(BUILD_DIR) install *.so *.dylib

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