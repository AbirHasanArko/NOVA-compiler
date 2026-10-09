# NOVA Compiler Makefile
# Reference Compiler Toolchain: C++17, Flex, Bison

# Ensure MSYS2 binaries (sh, flex, bison, mkdir, cp, rm) are in PATH
export PATH := C:/msys64/usr/bin:C:/msys64/ucrt64/bin:$(PATH)

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -Isrc

# Use MSYS2 sh for POSIX consistency if present
ifeq ($(OS),Windows_NT)
    ifneq ($(wildcard C:/msys64/usr/bin/sh.exe),)
        SHELL := C:/msys64/usr/bin/sh.exe
    endif
endif

PYTHON ?= python

TARGET := nova
BUILD_DIR := build
BIN_DIR := bin

SRCS := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all clean test help

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo [LD] $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@cp $@ $(BIN_DIR)/$@ 2>/dev/null || true

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	@echo [CXX] $<
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

test: $(TARGET)
	@echo [TEST] Executing NOVA Test Suite...
	$(PYTHON) scripts/test_runner.py

clean:
	@echo [CLEAN] Removing build artifacts...
	@$(PYTHON) -c "import shutil, os, glob; [shutil.rmtree(d, ignore_errors=True) for d in ['build', 'bin']]; [os.remove(f) for f in glob.glob('nova*') if os.path.isfile(f) and not f.endswith('.cpp') and not f.endswith('.md')]"

help:
	@echo "Available make targets:"
	@echo "  all     Build the NOVA compiler executable (default)"
	@echo "  test    Run the test suite via scripts/test_runner.py"
	@echo "  clean   Remove build objects and compiled binaries"
	@echo "  help    Display this help information"
