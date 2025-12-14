CC = gcc

POLICY_NAMES := fifo lifo sjf priority rr srt mlfq preemptive_priority

VALID_POLICIES :=

$(foreach policy,$(POLICY_NAMES),\
    $(if $(and $(wildcard src/policies/$(policy).c),\
               $(wildcard headers/policies/$(policy).h)),\
        $(eval VALID_POLICIES += $(policy))))

POLICY_DEFINES := $(foreach policy,$(VALID_POLICIES),\
    -DHAVE_$(shell echo $(policy) | tr '[:lower:]' '[:upper:]')_POLICY)

CFLAGS = -w -g -Iheaders $(POLICY_DEFINES)

TARGET = scheduler

TARGET = scheduler

ALL_SRCS = $(shell find src -name '*.c')

POSSIBLE_POLICY_SRCS = $(wildcard src/policies/*.c)
POSSIBLE_POLICY_SRCS := $(filter-out src/policies/policies.c, $(POSSIBLE_POLICY_SRCS))

VALID_POLICY_SRCS = $(foreach policy,$(VALID_POLICIES),src/policies/$(policy).c)

IGNORED_POLICY_SRCS = $(filter-out $(VALID_POLICY_SRCS), $(POSSIBLE_POLICY_SRCS))

SRCS = $(filter-out $(IGNORED_POLICY_SRCS), $(ALL_SRCS))

OBJS = $(SRCS:%.c=build/%.o)
TEST_SRCS = tests/test_parser.c tests/test_fifo_policy.c tests/test_lifo_policy.c tests/test_priority_policy.c tests/test_sjf_policy.c tests/test_parser_logic.c tests/test_policy_interface.c tests/test_scheduler_engine.c
TEST_TARGETS = $(TEST_SRCS:tests/%.c=build/%)

all: $(TARGET)
	@echo "✅  Build complete: ./${TARGET}"

tui: tui_scheduler
	@echo "✅  TUI build complete: ./tui_scheduler"

gui: gui_scheduler
	@echo "✅  GUI build complete: ./gui_scheduler"

$(TARGET): $(filter-out build/tests/test_runner.o build/src/main/tui_scheduler.o build/src/main/gui_scheduler.o, $(OBJS))
	@echo "🔗  LD | Linking main application: $@"
	@$(CC) $^ -o $@

tui_scheduler: $(filter-out build/src/main/main.o build/src/main/gui_scheduler.o build/tests/test_runner.o, $(OBJS))
	@echo "🔗  LD | Linking TUI application: $@"
	@$(CC) $^ -o $@ -lncurses

gui_scheduler: $(filter-out build/src/main/main.o build/src/main/tui_scheduler.o build/tests/test_runner.o, $(OBJS))
	@echo "🔗  LD | Linking GUI application: $@"
	@$(CC) $^ -o $@ `pkg-config --cflags --libs gtk+-3.0`

build/%.o: %.c
	@echo "🎨  CC | Compiling $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

build/src/main/gui_scheduler.o: src/main/gui_scheduler.c
	@echo "🎨  CC | Compiling $< (with GTK)"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0` -c $< -o $@

test: $(TEST_TARGETS)
	@echo "🚀  Running all tests..."
	@for test_exe in $(TEST_TARGETS); do \
		echo "--- Running test: $$test_exe ---"; \
		./$$test_exe && printf "  ✅ PASSED\n" || exit 1; \
		echo ""; \
	done
	@echo "✨  All tests passed."

build/test_%: tests/test_%.c $(filter-out build/src/main/main.o, $(OBJS))
	@echo "🔗  LD | Linking test: $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $^ -o $@


run: all
	@echo "🚀  Running application..."
	./$(TARGET)

clean:
	@echo "🗑️  CLEAN | Removing build artifacts..."
	@rm -f $(TARGET) tui_scheduler gui_scheduler
	@rm -rf build
	@echo "✨  Project is clean."

rebuild: clean all

.PHONY: all run clean rebuild test tui gui