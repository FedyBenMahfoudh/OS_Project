# ==============================================================================
# =                            Compiler and Flags                            =
# ==============================================================================
CC = gcc
# -Iheaders allows #include <...h> to search in the headers/ directory
# -Wall enables all warnings, -g adds debug symbols
CFLAGS = -Wall -g -Iheaders

# ==============================================================================
# =                              File Discovery                              =
# ==============================================================================
# The final executable name for the main application
TARGET = scheduler

# Automatically find all .c files in the src/ directory and its subdirectories
SRCS = $(shell find src -name '*.c')
# Generate corresponding object file names, placing them in a 'build/' directory
OBJS = $(SRCS:%.c=build/%.o)

# --- Test Specific Files ---
# Find all test files in the tests/ directory
TEST_SRCS = tests/test_parser.c tests/test_fifo_policy.c tests/test_lifo_policy.c tests/test_priority_policy.c tests/test_sjf_policy.c tests/test_parser_logic.c tests/test_policy_interface.c tests/test_scheduler_engine.c
# Create a list of test executables that will be placed in the 'build/' directory
TEST_TARGETS = $(TEST_SRCS:tests/%.c=build/%)

# ==============================================================================
# =                               Main Targets                               =
# ==============================================================================
# The default target, 'all', builds the main application executable
all: $(TARGET)
	@echo "✅  Build complete: ./${TARGET}"

# Rule to link the final application executable
# It depends on all object files except the test runner's object file
$(TARGET): $(filter-out build/tests/test_runner.o, $(OBJS))
	@echo "🔗  LD | Linking main application: $@"
	@$(CC) $^ -o $@

# Pattern rule to compile any .c file into a .o file in the build directory
build/%.o: %.c
	@echo "🎨  CC | Compiling $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# ==============================================================================
# =                               Test Targets                               =
# ==============================================================================
# The 'test' target will run all discovered tests, and exit immediately on failure
test: $(TEST_TARGETS)
	@echo "🚀  Running all tests..."
	@for test_exe in $(TEST_TARGETS); do \
		echo "--- Running test: $$test_exe ---"; \
		./$$test_exe && printf "  ✅ PASSED\n" || exit 1; \
		echo ""; \
	done
	@echo "✨  All tests passed."

# Rule to build a test executable.
# A test executable depends on its own .c file and ALL application .o files (excluding main.o)
build/test_%: tests/test_%.c $(filter-out build/src/main/main.o, $(OBJS))
	@echo "🔗  LD | Linking test: $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $^ -o $@


# ==============================================================================
# =                              Utility Rules                               =
# ==============================================================================
# 'make run' will compile and run the main application
run: all
	@echo "🚀  Running application..."
	./$(TARGET)

# 'make clean' removes the executable and the build directory
clean:
	@echo "🗑️  CLEAN | Removing build artifacts..."
	@rm -f $(TARGET)
	@rm -rf build
	@echo "✨  Project is clean."

# 'make rebuild' will clean and then build the main application
rebuild: clean all

# Declare targets that are not files
.PHONY: all run clean rebuild test