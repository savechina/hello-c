# ==============================================================================
# Makefile for C projects on macOS
# Supports multiple .c files, .h headers, and module dependencies.
# Includes 'make help' for usage instructions.
# ==============================================================================

# --- Project Configuration ---
# Name of the final executable
TARGET      := hello
# Directory containing source files (.c)
SRC_DIR     := src
# New: Directory for public header files
INCLUDE_DIR := include
# Project-level third-party dependencies (generic, for future use)
VENDOR_DIR      := vendor
VENDOR_SOURCES  := $(wildcard $(VENDOR_DIR)/**/*.c)
VENDOR_OBJS     := $(patsubst $(VENDOR_DIR)/%.c,$(BUILD_DIR)/vendor/%.o,$(VENDOR_SOURCES))
# Build output root directory
BUILD_DIR   := build
# Directory for intermediate object files (.o)
OBJ_DIR     := $(BUILD_DIR)/obj
# Directory for the final executable
BIN_DIR     := $(BUILD_DIR)/bin

# Find all .c source files in SRC_DIR (including _sample.c files)
# Per hello-rust convention: all samples compiled into single 'hello' binary
SOURCES     := $(wildcard $(SRC_DIR)/**.c $(SRC_DIR)/**/**.c)

# Generate corresponding .o object files based on sources and OBJ_DIR
# e.g., src/main.c -> build/obj/main.o
OBJECTS     := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

# --- DEBUGGING LINES ---
$(info DEBUG: SOURCES found: $(SOURCES))
$(info DEBUG: OBJECTS found: $(OBJECTS))
# --- END DEBUGGING LINES ---

# --- Compiler & Flags ---
# Operation System
UNAME_S := $(shell uname -s)

#CROSS_COMPILE ?= arm-linux-gnueabihf-
CROSS_COMPILE ?=
CC          := $(CROSS_COMPILE)gcc            # C compiler on macOS (default to clang)
# CFLAGS: Compiler flags for compilation
# -Wall: Enable all standard warnings
# -Wextra: Enable extra warnings
# -g: Include debugging information (for gdb/lldb)
# -O2: Optimization level 2
# -std=c11: Use C11 standard
# -MMD: Generate dependency files (.d) automatically
CFLAGS      := -Wall -Wextra -Werror -g -O2 -std=c17 -MMD -I$(INCLUDE_DIR)

# Add all source directories (including subdirectories) for header search too
# This is useful if source files include headers from other source subdirs.
INC_DIRS    := $(sort $(dir $(SOURCES)))
CFLAGS      += $(addprefix -I,$(INC_DIRS))

# LDFLAGS: Linker flags for linking (e.g., libraries)
# NOTE: -lsqlite3 placed at end; platform blocks may prepend -L paths before it
LDFLAGS     := -lm -pthread

ifeq ($(UNAME_S),SunOS)
    LDFLAGS  += -lkstat
    CFLAGS  += -D__sun__
endif

ifeq ($(UNAME_S),Linux)
    # Enable POSIX.1-2008 + BSD extensions on glibc with strict -std=c17
    CFLAGS  += -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
endif

ifeq ($(UNAME_S),FreeBSD)
    # FreeBSD installs third-party libs (e.g. sqlite3) under /usr/local
    LDFLAGS := -L/usr/local/lib $(LDFLAGS)
    CFLAGS  += -I/usr/local/include
endif

# Common libraries (appended after platform-specific -L paths)
LDFLAGS += -lsqlite3

# ============================================================
# Test configuration (Unity test framework)
# ============================================================
TEST_DIR          := test
TEST_VENDOR_DIR   := $(TEST_DIR)/vendor
UNITY_DIR         := $(TEST_VENDOR_DIR)/unity

# Test vendor objects (generic - captures all test/vendor/**/*.c)
TEST_VENDOR_SOURCES := $(wildcard $(TEST_VENDOR_DIR)/**/*.c)
TEST_VENDOR_OBJS    := $(patsubst $(TEST_VENDOR_DIR)/%.c,$(BUILD_DIR)/test-vendor/%.o,$(TEST_VENDOR_SOURCES))
UNITY_OBJ           := $(BUILD_DIR)/test-vendor/unity/unity.o

TEST_CFLAGS       := $(CFLAGS) -I$(UNITY_DIR) -DUNITY_OUTPUT_COLOR -DUNITY_SUPPORT_VARIADIC_MACROS

# Test sources: exclude vendor/ directory (Unity/CMock are compiled separately)
TEST_SOURCES_RAW := $(wildcard $(TEST_DIR)/**.c $(TEST_DIR)/**/**.c)
TEST_SOURCES     := $(filter-out $(TEST_VENDOR_DIR)/%, $(TEST_SOURCES_RAW))
TEST_BINS        := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/test/%,$(TEST_SOURCES))

$(BUILD_DIR)/test:
	@mkdir -p $@

# --- Directories Setup ---
# Create output directories if they don't exist
OBJ_SUBDIRS := $(sort $(dir $(OBJECTS)))
DIRS        := $(OBJ_SUBDIRS) $(BIN_DIR)

$(info DEBUG: DIRS found: $(DIRS))

# --- Include Dependency Files (.d) ---
-include $(OBJECTS:.o=.d)

# --- Default Target ---
default: help
.DEFAULT_GOAL: help

.PHONY: all
all: build

# ---Build  ---
.PHONY: build
build: $(DIRS) $(BIN_DIR)/$(TARGET)

# --- Project Vendor Compilation Rule ---
$(BUILD_DIR)/vendor/%.o: $(VENDOR_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Linker Rule (Build Executable) ---
$(BIN_DIR)/$(TARGET): $(OBJECTS) $(VENDOR_OBJS)
	@echo "Linking executable: $@"
	$(CC) $(OBJECTS) $(VENDOR_OBJS) $(LDFLAGS) -o $@

# --- Compilation Rule (Build Object Files) ---
# Compiles each .c file into a .o file in OBJ_DIR
# $<: The name of the first prerequisite (the .c file)
# $@: The name of the target (the .o file)
# -c: Compile only, do not link
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $< to $@ ..."
	$(CC) $(CFLAGS) -c $< -o $@

# --- Directory Creation Rule ---
$(DIRS):
	@mkdir -p $@

# --- Run Target ---
.PHONY: run
run:
	@echo "Running project..."
	$(BIN_DIR)/$(TARGET)
	@echo "Running complete"

# ==============================================================================
# --- Sample Targets (hello-rust convention) ---
# ==============================================================================
# All _sample.c files are compiled into the single 'hello' binary via glob.
# Usage:
#   make sample CHAPTER=<name>  # rebuild and run (full binary includes all samples)
# ==============================================================================
.PHONY: sample
sample: build
	@echo "Running hello (all samples compiled)..."
	@echo "---"
	$(BIN_DIR)/$(TARGET)
	@echo "---"

.PHONY: sample-all
sample-all: build
	@echo "Running hello (full tutorial suite)..."
	@echo "---"
	$(BIN_DIR)/$(TARGET)
	@echo "---"

# ============================================================
# Test vendor compilation (generic rule for all test/vendor/**/*.c)
# ============================================================
$(BUILD_DIR)/test-vendor/%.o: $(TEST_VENDOR_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(BUILD_DIR)/test/%: $(TEST_DIR)/%.c $(TEST_VENDOR_OBJS) $(OBJ_DIR)/advance/calc.o | $(BUILD_DIR)/test
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -c $< -o $@.o
	$(CC) $(TEST_CFLAGS) $@.o $(TEST_VENDOR_OBJS) $(OBJ_DIR)/advance/calc.o -o $@
	@echo "Test binary built: $@"

# --- Test Target ---
.PHONY: test
test: $(TEST_BINS)
	@failed=0; \
	for t in $(TEST_BINS); do \
		./$$t || { failed=1; }; \
	done; \
	[ $$failed -eq 0 ]

# --- Valgrind Test Target ---
.PHONY: test-valgrind
test-valgrind: $(TEST_BINS)
	@failed=0; \
	for t in $(TEST_BINS); do \
		echo "Running valgrind on $$t..."; \
		valgrind --leak-check=full --error-exitcode=1 $$t || { failed=1; }; \
	done; \
	[ $$failed -eq 0 ]

# --- Clean Target ---
.PHONY: clean
clean:
	@echo "Cleaning project..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR) $(BUILD_DIR)/vendor/ $(BUILD_DIR)/test-vendor/ $(BUILD_DIR)/test/
	@rm -f $(OBJ_DIR)/*.d $(OBJ_DIR)/*/*.d 2>/dev/null || true
	@echo "Clean complete."

# --- Help Target ---
.PHONY: help
help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all        (default) Builds the final executable '$(TARGET)'."
	@echo "  build      Builds the final executable '$(TARGET)'."
	@echo "  run        Run the final executable '$(TARGET)'."
	@echo "  sample CHAPTER=<name>  Rebuild and run (all _sample.c files included)."
	@echo "  sample-all             Same as 'run' — full tutorial suite."
	@echo "  clean      Removes all compiled objects and the executable."
	@echo "  help       Displays this help message."
	@echo ""
	@echo "Configuration:"
	@echo "  Executable     : $(TARGET)"
	@echo "  Source Dir     : $(SRC_DIR)"
	@echo "  Include Dir    : $(INCLUDE_DIR)"
	@echo "  Build Dir      : $(BUILD_DIR)"
	@echo "  Object Dir     : $(OBJ_DIR)"
	@echo "  Binary Dir     : $(BIN_DIR)"
	@echo "  Project Vendor : $(VENDOR_DIR)"
	@echo "  Test Dir       : $(TEST_DIR)"
	@echo "  Test Vendor    : $(TEST_VENDOR_DIR)"
	@echo "  Compiler       : $(CC)"
	@echo "  CFLAGS         : $(CFLAGS)"
	@echo "  LDFLAGS        : $(LDFLAGS)"

# --- End of Makefile ---
