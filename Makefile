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
# Directory for intermediate object files (.o)
OBJ_DIR     := build/obj
# Directory for the final executable
BIN_DIR     := build/bin

# Find all .c source files in SRC_DIR
SOURCES     := $(wildcard $(SRC_DIR)/**.c $(SRC_DIR)/**/**.c)

# Generate corresponding .o object files based on sources and OBJ_DIR
# e.g., src/main.c -> build/obj/main.o
OBJECTS     := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

# --- DEBUGGING LINES ---
$(info DEBUG: SRC_DIR is $(SRC_DIR))
$(info DEBUG: Wildcard pattern is $(SRC_DIR)/**.c $(SRC_DIR)/**/**.c)
$(info DEBUG: SOURCES found: $(SOURCES))
$(info DEBUG: OBJECTS found: $(OBJECTS))
# --- END DEBUGGING LINES ---

# --- Compiler & Flags ---
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
CFLAGS      := -Wall -Wextra -g -O2 -std=c17 -MMD -I$(INCLUDE_DIR)

# Add all source directories (including subdirectories) for header search too
# This is useful if source files include headers from other source subdirs.
INC_DIRS    := $(sort $(dir $(SOURCES)))
CFLAGS      += $(addprefix -I,$(INC_DIRS))

# LDFLAGS: Linker flags for linking (e.g., libraries)
LDFLAGS     :=

# --- Directories Setup ---
# Create output directories if they don't exist
OBJ_SUBDIRS := $(sort $(dir $(OBJECTS)))
DIRS        := $(OBJ_SUBDIRS) $(OBJ_DIR) $(BIN_DIR)

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
build: clean $(DIRS) $(BIN_DIR)/$(TARGET)

# --- Linker Rule (Build Executable) ---
$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@echo "Linking executable: $@"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# --- Compilation Rule (Build Object Files) ---
# Compiles each .c file into a .o file in OBJ_DIR
# $<: The name of the first prerequisite (the .c file)
# $@: The name of the target (the .o file)
# -c: Compile only, do not link
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $< to $@ ..."
	$(CC) $(CFLAGS) -c $< -o $@

# --- Include Dependency Files (.d) ---
# This line tells Make to include the automatically generated .d files.
# These files contain rules for header file dependencies,
# so if a header changes, the corresponding .c file will be recompiled.
-include $(OBJECTS:.o=.d)

# --- Directory Creation Rule ---
$(DIRS):
	@mkdir -p $@

# --- Run Target ---
.PHONY: run
run:
	@echo "Running project..."
	$(BIN_DIR)/$(TARGET)
	@echo "Running complete"

# --- Clean Target ---
.PHONY: clean
clean:
	@echo "Cleaning project..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
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
	@echo "  clean      Removes all compiled objects and the executable."
	@echo "  help       Displays this help message."
	@echo ""
	@echo "Configuration:"
	@echo "  Executable: $(TARGET)"
	@echo "  Source Dir: $(SRC_DIR)"
	@echo "  Include Dir: $(INCLUDE_DIR)"
	@echo "  Object Dir: $(OBJ_DIR)"
	@echo "  Binary Dir: $(BIN_DIR)"
	@echo "  Compiler  : $(CC)"
	@echo "  CFLAGS    : $(CFLAGS)"
	@echo "  LDFLAGS   : $(LDFLAGS)"

# --- End of Makefile ---
