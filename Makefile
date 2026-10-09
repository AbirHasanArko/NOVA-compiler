# NOVA Compiler Makefile
# Reference Compiler Toolchain: C++17, Flex, Bison

# Ensure MSYS2 binaries (sh, flex, bison, mkdir, cp, rm) are in PATH
export PATH := C:/msys64/usr/bin:C:/msys64/ucrt64/bin:$(PATH)

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -Isrc

# Path resolution for Flex
FLEX_BIN := $(shell which flex 2>/dev/null || echo C:/msys64/usr/bin/flex.exe)
PYTHON ?= python

# Use MSYS2 sh for POSIX consistency if present
ifeq ($(OS),Windows_NT)
    ifneq ($(wildcard C:/msys64/usr/bin/sh.exe),)
        SHELL := C:/msys64/usr/bin/sh.exe
    endif
endif

TARGET := nova
BUILD_DIR := build
BIN_DIR := bin

# Source files
GEN_SRCS := src/lexer/lex.yy.cpp
SRCS := src/main.cpp src/lexer/lexer.cpp $(GEN_SRCS)
OBJS := $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all clean test help

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo [LD] $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@$(PYTHON) -c "import shutil; shutil.copy('$(TARGET)', '$(BIN_DIR)/$(TARGET)')" 2>/dev/null || true

# Flex generation rule
src/lexer/lex.yy.cpp: src/lexer/scanner.l
	@mkdir -p src/lexer
	@echo [FLEX] $<
	$(FLEX_BIN) -o $@ $<

# C++ Compilation rule
$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo [CXX] $<
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/lexer/lex.yy.o: src/lexer/lex.yy.cpp

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

test: $(TARGET)
	@echo [TEST] Executing NOVA Test Suite...
	$(PYTHON) scripts/test_runner.py

clean:
	@echo [CLEAN] Removing build artifacts...
	@$(PYTHON) -c "import shutil, os, glob; [shutil.rmtree(d, ignore_errors=True) for d in ['build', 'bin']]; [os.remove(f) for f in ['src/lexer/lex.yy.cpp'] if os.path.exists(f)]; [os.remove(f) for f in glob.glob('nova*') if os.path.isfile(f) and not f.endswith('.cpp') and not f.endswith('.md')]"

help:
	@echo "Available make targets:"
	@echo "  all     Build the NOVA compiler executable (default)"
	@echo "  test    Run the test suite via scripts/test_runner.py"
	@echo "  clean   Remove build objects and compiled binaries"
	@echo "  help    Display this help information"
