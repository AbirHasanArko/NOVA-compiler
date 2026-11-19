# NOVA Compiler Makefile
# Reference Compiler Toolchain: C++17, Flex, Bison

# Ensure MSYS2 binaries (sh, flex, bison, mkdir, cp, rm) are in PATH
export PATH := C:/msys64/usr/bin:C:/msys64/ucrt64/bin:$(PATH)

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -Isrc

# Path resolution for Flex and Bison
FLEX_BIN := $(shell which flex 2>/dev/null || echo C:/msys64/usr/bin/flex.exe)
BISON_BIN := $(shell which bison 2>/dev/null || echo C:/msys64/usr/bin/bison.exe)
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

# Source and Object files
GEN_SRCS := src/lexer/lex.yy.c src/parser/parser.tab.cpp src/parser/parser.tab.hpp
OBJS := $(BUILD_DIR)/main.o $(BUILD_DIR)/lexer/lexer.o $(BUILD_DIR)/lexer/lex.yy.o $(BUILD_DIR)/parser/parser.o $(BUILD_DIR)/parser/parser.tab.o $(BUILD_DIR)/ast/ast_printer.o $(BUILD_DIR)/diagnostics/diagnostic.o $(BUILD_DIR)/semantic/symbol_table.o $(BUILD_DIR)/semantic/symbol_resolver.o

.PHONY: all clean test help

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo [LD] $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@$(PYTHON) -c "import shutil; shutil.copy('$(TARGET)', '$(BIN_DIR)/$(TARGET)')" 2>/dev/null || true

# Flex generation rule (producing lex.yy.c)
src/lexer/lex.yy.c: src/lexer/scanner.l
	@mkdir -p src/lexer
	@echo [FLEX] $<
	$(FLEX_BIN) -o $@ $<

# Bison generation rule (producing parser.tab.cpp & parser.tab.hpp)
src/parser/parser.tab.cpp src/parser/parser.tab.hpp: src/parser/parser.y
	@mkdir -p src/parser
	@echo [BISON] $<
	$(BISON_BIN) -Wall -d -o src/parser/parser.tab.cpp $<

# Generated header dependencies
$(BUILD_DIR)/main.o: src/main.cpp src/ast/ast_node.hpp src/ast/ast_printer.hpp src/ast/ast_visitor.hpp src/diagnostics/diagnostic.hpp src/semantic/symbol.hpp src/semantic/symbol_resolver.hpp
$(BUILD_DIR)/ast/ast_printer.o: src/ast/ast_printer.cpp src/ast/ast_printer.hpp src/ast/ast_node.hpp src/ast/ast_visitor.hpp
$(BUILD_DIR)/diagnostics/diagnostic.o: src/diagnostics/diagnostic.cpp src/diagnostics/diagnostic.hpp src/lexer/token.hpp
$(BUILD_DIR)/semantic/symbol_table.o: src/semantic/symbol_table.cpp src/semantic/symbol.hpp src/ast/ast_node.hpp src/lexer/token.hpp
$(BUILD_DIR)/semantic/symbol_resolver.o: src/semantic/symbol_resolver.cpp src/semantic/symbol_resolver.hpp src/semantic/symbol.hpp src/diagnostics/diagnostic.hpp src/ast/ast_node.hpp src/ast/ast_visitor.hpp
$(BUILD_DIR)/parser/parser.o: src/parser/parser.cpp src/parser/parser.tab.hpp src/ast/ast_node.hpp src/ast/ast_visitor.hpp
$(BUILD_DIR)/parser/parser.tab.o: src/parser/parser.tab.cpp src/parser/parser.tab.hpp src/ast/ast_node.hpp src/ast/ast_visitor.hpp

# C++ Compilation rule for standard .cpp sources
$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo [CXX] $<
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilation rule for Flex generated .c file (compiled as C++)
$(BUILD_DIR)/lexer/lex.yy.o: src/lexer/lex.yy.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo [CXX] $<
	$(CXX) $(CXXFLAGS) -x c++ -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

test: $(TARGET)
	@echo [TEST] Executing NOVA Test Suite...
	$(PYTHON) scripts/test_runner.py

counter: $(BIN_DIR)/keyword_counter

$(BIN_DIR)/keyword_counter: tools/keyword_counter/counter.l | $(BIN_DIR)
	@mkdir -p build/counter
	@echo [FLEX] $<
	$(FLEX_BIN) -o build/counter/counter.yy.c $<
	@echo [CXX] build/counter/counter.yy.c
	$(CXX) $(CXXFLAGS) -o $@ build/counter/counter.yy.c

count-keywords: $(BIN_DIR)/keyword_counter
	@$(BIN_DIR)/keyword_counter examples/demo.nova keyword_counts.txt

clean:
	@echo [CLEAN] Removing build artifacts...
	@$(PYTHON) -c "import shutil, os, glob; [shutil.rmtree(d, ignore_errors=True) for d in ['build', 'bin']]; [os.remove(f) for f in ['src/lexer/lex.yy.c', 'src/lexer/lex.yy.cpp', 'src/parser/parser.tab.cpp', 'src/parser/parser.tab.hpp', 'keyword_counts.txt'] if os.path.exists(f)]; [os.remove(f) for f in glob.glob('nova*') if os.path.isfile(f) and not f.endswith('.cpp') and not f.endswith('.md')]"

help:
	@echo "Available make targets:"
	@echo "  all            Build the NOVA compiler executable (default)"
	@echo "  counter        Build the standalone Flex keyword counter tool"
	@echo "  count-keywords Run the keyword counter on examples/demo.nova"
	@echo "  test           Run the test suite via scripts/test_runner.py"
	@echo "  clean          Remove build objects and compiled binaries"
	@echo "  help           Display this help information"
