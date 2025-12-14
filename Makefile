# ==============================================================================
# =                            Compiler and Flags                            =
# ==============================================================================
CC = gcc

# ==============================================================================
# =                        Dynamic Policy Detection                          =
# ==============================================================================
# List of all known policy names in the project
POLICY_NAMES := fifo lifo sjf priority rr srt mlfq preemptive_priority

# Initialize empty list for valid policies
VALID_POLICIES :=

# For each policy, check if BOTH .h and .c files exist
# Only include the policy if both files are present
$(foreach policy,$(POLICY_NAMES),\
    $(if $(and $(wildcard src/policies/$(policy).c),\
               $(wildcard headers/policies/$(policy).h)),\
        $(eval VALID_POLICIES += $(policy))))

# Generate compilation flags for each valid policy
# Example: fifo -> -DHAVE_FIFO_POLICY
POLICY_DEFINES := $(foreach policy,$(VALID_POLICIES),\
    -DHAVE_$(shell echo $(policy) | tr '[:lower:]' '[:upper:]')_POLICY)

# -Iheaders allows #include <...h> to search in the headers/ directory
# -Wall enables all warnings, -g adds debug symbols
# POLICY_DEFINES are automatically generated based on available policy files
CFLAGS = -w -g -Iheaders $(POLICY_DEFINES)

# ==============================================================================
# =                              File Discovery                              =
# ==============================================================================
# The final executable name for the main application
TARGET = scheduler

# ==============================================================================
# =                              File Discovery                              =
# ==============================================================================
# The final executable name for the main application
TARGET = scheduler

# Automatically find all .c files in the src/ directory and its subdirectories
ALL_SRCS = $(shell find src -name '*.c')

# --- Filter out Invalid Policies ---
# Identify all policy source files (excluding policies.c which is the registrar)
POSSIBLE_POLICY_SRCS = $(wildcard src/policies/*.c)
POSSIBLE_POLICY_SRCS := $(filter-out src/policies/policies.c, $(POSSIBLE_POLICY_SRCS))

# Identify which ones are VALID (based on VALID_POLICIES computed above)
# We reconstruct the path: src/policies/name.c
VALID_POLICY_SRCS = $(foreach policy,$(VALID_POLICIES),src/policies/$(policy).c)

# Determine which sources to EXCLUDE (Present on disk but not Valid)
# This handles the case where .c exists but .h is missing
IGNORED_POLICY_SRCS = $(filter-out $(VALID_POLICY_SRCS), $(POSSIBLE_POLICY_SRCS))

# Final SRCS list: All sources minus the ignored policy files
SRCS = $(filter-out $(IGNORED_POLICY_SRCS), $(ALL_SRCS))

# Generate corresponding object file names, placing them in a 'build/' directory
OBJS = $(SRCS:%.c=build/%.o)
TEST_SRCS = tests/test_parser.c tests/test_fifo_policy.c tests/test_lifo_policy.c tests/test_priority_policy.c tests/test_sjf_policy.c tests/test_parser_logic.c tests/test_policy_interface.c tests/test_scheduler_engine.c
# Create a list of test executables that will be placed in the 'build/' directory
TEST_TARGETS = $(TEST_SRCS:tests/%.c=build/%)

# ==============================================================================
# =                               Main Targets                               =
# ==============================================================================
# The default target, 'all', builds the main application executable
all: $(TARGET)
	@echo "✅  Build complete: ./${TARGET}"

# Build TUI version
tui: tui_scheduler
	@echo "✅  TUI build complete: ./tui_scheduler"

# Build GUI version  
gui: gui_scheduler
	@echo "✅  GUI build complete: ./gui_scheduler"

# Rule to link the final application executable
# It depends on all object files except the test runner's object file
$(TARGET): $(filter-out build/tests/test_runner.o build/src/main/tui_scheduler.o build/src/main/gui_scheduler.o, $(OBJS))
	@echo "🔗  LD | Linking main application: $@"
	@$(CC) $^ -o $@

# Rule to link the TUI scheduler with ncurses
tui_scheduler: $(filter-out build/src/main/main.o build/src/main/gui_scheduler.o build/tests/test_runner.o, $(OBJS))
	@echo "🔗  LD | Linking TUI application: $@"
	@$(CC) $^ -o $@ -lncurses

# Rule to link the GUI scheduler with GTK
gui_scheduler: $(filter-out build/src/main/main.o build/src/main/tui_scheduler.o build/tests/test_runner.o, $(OBJS))
	@echo "🔗  LD | Linking GUI application: $@"
	@$(CC) $^ -o $@ `pkg-config --cflags --libs gtk+-3.0`

# Pattern rule to compile any .c file into a .o file in the build directory
build/%.o: %.c
	@echo "🎨  CC | Compiling $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Special rule for GUI scheduler - needs GTK flags
build/src/main/gui_scheduler.o: src/main/gui_scheduler.c
	@echo "🎨  CC | Compiling $< (with GTK)"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0` -c $< -o $@

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
	@rm -f $(TARGET) tui_scheduler gui_scheduler
	@rm -rf build
	@echo "✨  Project is clean."

# 'make rebuild' will clean and then build the main application
rebuild: clean all

# Declare targets that are not files
.PHONY: all run clean rebuild test tui gui install-dependencies

install-dependencies:
	@echo "Installing system dependencies (requires sudo)..."
	sudo apt-get update
	sudo apt-get install -y build-essential git libncurses5-dev libgtk-3-dev
	@echo "Dependencies installed successfully."