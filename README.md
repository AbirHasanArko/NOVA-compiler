# NOVA Programming Language & Compiler

> **NOVA**: A compiled, statically-typed programming language engineered with a **Cosmic Telemetry & Systems Computing** aesthetic. Built with Flex, Bison, and C++17.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](#)
[![Phase](https://img.shields.io/badge/phase-Phase%206%20Complete-blue)](#)
[![License](https://img.shields.io/badge/license-MIT-informational)](#)

---

## Table of Contents
- [1. Overview & Linguistic Philosophy](#1-overview--linguistic-philosophy)
- [2. Project Status & Completed Milestones](#2-project-status--completed-milestones)
  - [Phase-by-Phase Files & Verification Log](#phase-by-phase-files--verification-log)
- [3. Prerequisites & Toolchain Setup](#3-prerequisites--toolchain-setup)
  - [Windows (MSYS2)](#windows-msys2)
  - [Linux / macOS](#linux--macos)
- [4. Build & Compilation Commands](#4-build--compilation-commands)
- [5. Compiler CLI Usage](#5-compiler-cli-usage)
- [6. Automated Testing Suite](#6-automated-testing-suite)
- [7. Editor Syntax Highlighting Extension](#7-editor-syntax-highlighting-extension)
  - [Installation in Antigravity IDE](#installation-in-antigravity-ide)
  - [Installation in VS Code / Cursor](#installation-in-vs-code--cursor)
  - [Reloading the Editor](#reloading-the-editor)
- [8. Keyword Counter Tool](#8-keyword-counter-tool)
- [9. Language Quick Reference & Cheat Sheet](#9-language-quick-reference--cheat-sheet)
- [10. Repository Structure](#10-repository-structure)
- [11. Roadmap (Upcoming Phases)](#11-roadmap-upcoming-phases)

---

## 1. Overview & Linguistic Philosophy

NOVA combines rigorous compiler construction theory (LALR(1) grammars, modular AST, static type checking, contract verification) with a distinctive **space mission telemetry** syntax:

- **Execution Capsule (`ignite do ... end`)**: The root execution block of every program.
- **Routines (`routine ... ret`)**: High-efficiency procedural routines with return types and preconditions.
- **Fixed-Size Arrays (`T[N]`)**: First-class contiguous compound types with zero-based indexing `arr[i]` and literals `[1, 2, 3]`.
- **State Classification (`const` vs `flux`)**:
  - `const`: Immutable state; mandatory initializer; cannot be reassigned or mutated.
  - `flux`: Dynamic mutable state; allows runtime reassignment and element mutation.
- **Execution Cycles**:
  - `orbit <idx> in <start>..<end> [step <s>] do ... end`: Counted stepped range loop (`for`).
  - `cruise <condition> do ... end`: Telemetry condition loop (`while`).
  - `halt;` and `skip;`: Loop interruption (`break` and `continue`).
- **Detection Branching (`detect` / `redetect` / `fallback`)**:
  - `detect`: Initial condition check (`if`).
  - `redetect`: Subsequent branch check (`elif`).
  - `fallback`: Contingency fallback block (`else`).
- **Telemetry I/O**:
  - `transmit(<expr1>, <expr2>, ...);`: Multi-argument standard output stream supporting all types.
  - `receive(<flux_var>);`: Standard input scanner.
- **Contracts & Safety**:
  - `verify <cond>;`: Runtime invariant assertion.
  - `require <cond>;`: Procedure precondition validation.

---

## 2. Project Status & Completed Milestones

| Milestone | Status | Description |
|---|---|---|
| **Phase 0: Specification & Setup** | Completed | Authoritative language spec, LALR(1) grammar, compiler architecture, testing protocol, research specs, and agent rules. |
| **Phase 1: Project Bootstrap** | Completed | Cross-platform `Makefile`, CLI driver (`--help`, `--version`, flags), and 5-state automated Python test harness. |
| **Phase 2: Flex Lexer** | Completed | Token model, Flex scanner, C++ token stream driver, `--tokens` flag, demo program, keyword counter, editor extension, and lexer tests. |
| **Phase 3: Bison Parser** | Completed | Pure reentrant LALR(1) Bison parser (`0` conflicts), statement-level error recovery, `--parse` flag, and parser test suite. |
| **Phase 4: Abstract Syntax Tree (AST)** | Completed | Polymorphic AST node hierarchy (24 concrete nodes), `ASTVisitor`, `ASTPrinter`, Bison AST construction, `--ast` flag, and AST tests. |
| **Phase 5: Expressions & Precedence** | Completed | Verified 9-level operator precedence, associativity (`**`, unary `-`/`not`, arithmetic, comparison, logical), array indexing, and complex equations. |
| **Phase 6: Variables & Symbol Table** | Completed | `DiagnosticEngine`, lexical `Scope`/`SymbolTable`, `SymbolResolver` (`const`/`flux`/array/`orbit` mutability, shadowing), `--symbols` flag, and semantic scope tests. |
| **Phase 7: Type Checking** | Next | Standalone `TypeChecker` pass enforcing `i32`, `f32`, `bool`, `char`, `str`, and `T[N]` type rules and bounds. |

### Phase-by-Phase Files & Verification Log

#### Phase 0 — Specification & Setup
- **Documentation & Spec Files**: [`AGENTS.md`](AGENTS.md), [`docs/language-spec.md`](docs/language-spec.md), [`docs/grammar.md`](docs/grammar.md), [`docs/architecture.md`](docs/architecture.md), [`docs/testing.md`](docs/testing.md), [`docs/research.md`](docs/research.md)
- **Test Files**: None (specification phase)
- **Result**: Authoritative 34-keyword language specification, LALR(1) BNF grammar, 5-state test protocol, and differential-fuzzing research plan established.

#### Phase 1 — Project Bootstrap & CLI Harness
- **Code & Build Files**: [`Makefile`](Makefile), [`.gitignore`](.gitignore), [`src/main.cpp`](src/main.cpp), [`scripts/test_runner.py`](scripts/test_runner.py)
- **Test Files**: [`tests/cli/valid_stub.nova`](tests/cli/valid_stub.nova), plus 6 built-in CLI tests in [`scripts/test_runner.py`](scripts/test_runner.py) (`--help`, `-h`, `--version`, no-args error, unrecognized flag error, missing file error)
- **Result**: Clean C++17 build via `make`; **7 / 7** bootstrap & CLI harness checks passing (`4 PASS`, `3 EXPECTED ERROR`).

#### Phase 2 — Flex Lexer, Tooling & Editor Integration
- **Code & Tooling Files**:
  - Lexer subsystem: [`src/lexer/token.hpp`](src/lexer/token.hpp), [`src/lexer/scanner.l`](src/lexer/scanner.l), [`src/lexer/lex.yy.c`](src/lexer/lex.yy.c), [`src/lexer/lexer.hpp`](src/lexer/lexer.hpp), [`src/lexer/lexer.cpp`](src/lexer/lexer.cpp)
  - Standalone keyword counter & demo: [`tools/keyword_counter/counter.l`](tools/keyword_counter/counter.l), [`examples/demo.nova`](examples/demo.nova)
  - Editor syntax extension: [`editors/vscode/package.json`](editors/vscode/package.json), [`editors/vscode/language-configuration.json`](editors/vscode/language-configuration.json), [`editors/vscode/syntaxes/nova.tmLanguage.json`](editors/vscode/syntaxes/nova.tmLanguage.json)
- **Generated Result Artifacts**: [`keyword_counts.txt`](keyword_counts.txt) (`34 / 34` unique keywords detected, `84` total keyword tokens in `examples/demo.nova`)
- **Test Files** (9 files in [`tests/lexer/`](tests/lexer/)):
  - Valid token tests: [`arrays.nova`](tests/lexer/arrays.nova), [`comments.nova`](tests/lexer/comments.nova), [`identifiers.nova`](tests/lexer/identifiers.nova), [`keywords.nova`](tests/lexer/keywords.nova), [`line_numbers.nova`](tests/lexer/line_numbers.nova), [`literals.nova`](tests/lexer/literals.nova), [`operators.nova`](tests/lexer/operators.nova)
  - Lexical diagnostic tests: [`invalid_character.nova`](tests/lexer/invalid_character.nova), [`invalid_literal.nova`](tests/lexer/invalid_literal.nova)
- **Result**: **9 / 9** lexer tests passing (`7 PASS`, `2 EXPECTED ERROR`); cumulative suite **16 / 16** passing with `--tokens` flag verified.

#### Phase 3 — Bison LALR(1) Parser
- **Code Files**: [`src/parser/parser.y`](src/parser/parser.y), [`src/parser/parser.hpp`](src/parser/parser.hpp), [`src/parser/parser.cpp`](src/parser/parser.cpp), plus Bison build rules in [`Makefile`](Makefile) and `--parse` / compile-mode parser wiring in [`src/main.cpp`](src/main.cpp) (generates `src/parser/parser.tab.cpp` and `src/parser/parser.tab.hpp` during `make`)
- **Test Files** (9 files in [`tests/parser/`](tests/parser/)):
  - Valid grammar tests: [`ignite.nova`](tests/parser/ignite.nova), [`routines_and_arrays.nova`](tests/parser/routines_and_arrays.nova), [`control_flow.nova`](tests/parser/control_flow.nova)
  - Syntax diagnostic & recovery tests: [`missing_end.nova`](tests/parser/missing_end.nova), [`missing_semicolon.nova`](tests/parser/missing_semicolon.nova), [`unexpected_token.nova`](tests/parser/unexpected_token.nova), [`missing_ignite.nova`](tests/parser/missing_ignite.nova), [`invalid_array_decl.nova`](tests/parser/invalid_array_decl.nova), [`error_recovery.nova`](tests/parser/error_recovery.nova)
- **Result**: GNU Bison (`-Wall -Wcounterexamples`) builds with **0 shift/reduce and 0 reduce/reduce conflicts**; **9 / 9** parser tests passing (`3 PASS`, `6 EXPECTED ERROR`), [`examples/demo.nova`](examples/demo.nova) passes `--parse`, cumulative suite **25 / 25** passing (`14 PASS`, `11 EXPECTED ERROR`, `0 FAIL`, `0 CRASH`).

#### Phase 4 — Abstract Syntax Tree (AST)
- **Code Files**: [`src/ast/ast_node.hpp`](src/ast/ast_node.hpp), [`src/ast/ast_visitor.hpp`](src/ast/ast_visitor.hpp), [`src/ast/ast_printer.hpp`](src/ast/ast_printer.hpp), [`src/ast/ast_printer.cpp`](src/ast/ast_printer.cpp), updated [`src/parser/parser.y`](src/parser/parser.y), [`src/parser/parser.hpp`](src/parser/parser.hpp), [`src/parser/parser.cpp`](src/parser/parser.cpp), [`src/main.cpp`](src/main.cpp) (`--ast`), and [`Makefile`](Makefile)
- **Test Files** (4 files in [`tests/ast/`](tests/ast/)):
  - Valid AST structure tests: [`declarations.nova`](tests/ast/declarations.nova), [`expressions.nova`](tests/ast/expressions.nova), [`control_flow.nova`](tests/ast/control_flow.nova)
  - Negative AST error handling test: [`invalid_syntax.nova`](tests/ast/invalid_syntax.nova)
- **Result**: Zero Bison conflicts (`-Wall`) and leak-free `%destructor` cleanup on syntax errors; **4 / 4** AST tests passing (`3 PASS`, `1 EXPECTED ERROR`), [`examples/demo.nova`](examples/demo.nova) dumps full AST cleanly via `--ast`, cumulative suite **29 / 29** passing (`17 PASS`, `12 EXPECTED ERROR`, `0 FAIL`, `0 CRASH`).

#### Phase 5 — Expressions & Precedence
- **Verified Grammar & Precedence Specification**: Audited all 9 precedence levels and shift/reduce conflict resolutions (`--report=all` via Bison `-Wall -Wcounterexamples`) in [`src/parser/parser.y`](src/parser/parser.y) against [`docs/language-spec.md`](docs/language-spec.md).
- **Test Files** (5 files in [`tests/expressions/`](tests/expressions/)):
  - Precedence & associativity tests: [`arithmetic_associativity.nova`](tests/expressions/arithmetic_associativity.nova), [`unary_and_power.nova`](tests/expressions/unary_and_power.nova), [`boolean_and_comparison.nova`](tests/expressions/boolean_and_comparison.nova), [`complex_equations_and_indexing.nova`](tests/expressions/complex_equations_and_indexing.nova)
  - Negative expression diagnostic test: [`invalid_expressions.nova`](tests/expressions/invalid_expressions.nova)
- **Result**: Verified right-associative `**`, `-a ** b` (`-(a ** b)`), left-associative arithmetic/relational/equality/logical operators, parenthesized overrides, routine calls, and chained indexing; **5 / 5** expression tests passing (`4 PASS`, `1 EXPECTED ERROR`), cumulative suite **34 / 34** passing (`21 PASS`, `13 EXPECTED ERROR`, `0 FAIL`, `0 CRASH`).

#### Phase 6 — Variables & Symbol Table
- **Code Files**: [`src/diagnostics/diagnostic.hpp`](src/diagnostics/diagnostic.hpp), [`src/diagnostics/diagnostic.cpp`](src/diagnostics/diagnostic.cpp), [`src/semantic/symbol.hpp`](src/semantic/symbol.hpp), [`src/semantic/symbol_table.cpp`](src/semantic/symbol_table.cpp), [`src/semantic/symbol_resolver.hpp`](src/semantic/symbol_resolver.hpp), [`src/semantic/symbol_resolver.cpp`](src/semantic/symbol_resolver.cpp), updated [`src/main.cpp`](src/main.cpp) (`--symbols`), and [`Makefile`](Makefile)
- **Test Files** (4 files in [`tests/semantic/`](tests/semantic/)):
  - Scoping & mutability resolution test: [`valid_scoping.nova`](tests/semantic/valid_scoping.nova)
  - Negative semantic tests: [`duplicate_decl.nova`](tests/semantic/duplicate_decl.nova), [`immutable_assign.nova`](tests/semantic/immutable_assign.nova), [`undeclared_symbol.nova`](tests/semantic/undeclared_symbol.nova)
- **Result**: Verified lexical block scoping, shadowing, same-scope redeclaration errors, undeclared identifier/routine errors, `const` / `const T[N]` / `orbit` index immutability checks across assignment and `receive`; **4 / 4** semantic symbol tests passing (`1 PASS`, `3 EXPECTED ERROR`), [`examples/demo.nova`](examples/demo.nova) passes `--symbols`, cumulative suite **38 / 38** passing (`22 PASS`, `16 EXPECTED ERROR`, `0 FAIL`, `0 CRASH`).

---

## 3. Prerequisites & Toolchain Setup

The project requires a modern C++17 compiler, Flex scanner generator, GNU Make, and Python 3.

### Windows (MSYS2)
The compiler build system is configured to auto-detect MSYS2 toolchain paths:
- **Compiler**: `g++` (GCC 16.2.0 or 14.x) via MSYS2 UCRT64 (`C:\msys64\ucrt64\bin\g++.exe`)
- **Build Automation**: GNU `make` 4.4 (`C:\msys64\ucrt64\bin\make.exe`)
- **Lexer Generator**: `flex` 2.6.4 (`C:\msys64\usr\bin\flex.exe`)
- **Parser Generator**: `bison` 3.8.2 (`C:\msys64\usr\bin\bison.exe`)
- **Scripting**: Python 3.10+ (for test orchestrator)

To install these via MSYS2 pacman:
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make flex bison python
```

### Linux / macOS
```bash
# Ubuntu / Debian
sudo apt-get install build-essential flex bison python3

# macOS (Homebrew)
brew install flex bison python3
```

---

## 4. Build & Compilation Commands

Build operations are managed through the root [`Makefile`](Makefile):

```bash
# 1. Compile the NOVA compiler binary (builds 'nova' / 'nova.exe')
make

# 2. Build and run the automated test suite
make test

# 3. Build and execute the Flex keyword frequency counter
make count-keywords

# 4. Clean all build objects, generated scanner C files, and binaries
make clean

# 5. Display available targets
make help
```

---

## 5. Compiler CLI Usage

```text
Usage: ./nova [options] <source_file.nova>

General Options:
  -h, --help              Display usage information and exit
  -v, --version           Display compiler version and edition

Pipeline Inspection Flags:
  --tokens <file>         Lexical analysis: scan and dump token stream with coordinates
  --parse <file>          Syntax analysis: parse and verify grammar
  --ast <file>            Syntax analysis: parse and dump AST
  --symbols <file>        Semantic analysis: dump symbol tables
  --contracts <file>      Verification: inspect contracts & preconditions (Phase 15)
  --baseline-ir <file>    IR: emit baseline Three-Address Code (Phase 17)
  --ir <file>             IR: emit contract-aware optimized TAC (Phase 16)
  -o <file>               Specify output artifact destination
```

### Example Commands:
```bash
# Inspect token stream of a NOVA program
./nova --tokens examples/demo.nova

# Validate syntax using the Bison LALR(1) parser
./nova --parse examples/demo.nova

# Dump Abstract Syntax Tree
./nova --ast examples/demo.nova

# Resolve scopes & dump lexical symbol table
./nova --symbols examples/demo.nova

# Scan a lexer test file
./nova --tokens tests/lexer/keywords.nova
```

---

## 6. Automated Testing Suite

The test harness [`scripts/test_runner.py`](scripts/test_runner.py) strictly validates test outputs against five mutually exclusive statuses:

- `PASS`: Valid test executed and output matched expectations.
- `EXPECTED ERROR`: Negative test produced the expected diagnostic message and error code.
- `UNEXPECTED ERROR`: Valid test failed, or invalid test failed with the wrong error code/message.
- `FAIL`: Output or exit code mismatch.
- `CRASH`: Compiler segfaulted, aborted, or crashed.

### Running the Suite:
```bash
make test
```

Current test coverage: **38 / 38 tests passing** (`22 PASS`, `16 EXPECTED ERROR`, `0 FAIL`, `0 CRASH`) across CLI flags, keywords, operators, literals, identifiers, comments, arrays, lexical coordinates/diagnostics, LALR(1) syntax validation, statement error recovery, AST construction, expression precedence, and lexical scoping/mutability symbol resolution.

---

## 7. Editor Syntax Highlighting Extension

A custom TextMate syntax highlighting package is included in [`editors/vscode/`](editors/vscode/):
- **Grammar**: [`editors/vscode/syntaxes/nova.tmLanguage.json`](editors/vscode/syntaxes/nova.tmLanguage.json)
- **Language Config**: [`editors/vscode/language-configuration.json`](editors/vscode/language-configuration.json) (bracket matching, auto-closing quotes, `do ... end` block indentation)
- **Manifest**: [`editors/vscode/package.json`](editors/vscode/package.json)

### Installation in Antigravity IDE:
The extension can be installed as a native built-in language:
```powershell
Copy-Item -Path "editors\vscode" -Destination "$env:LOCALAPPDATA\Programs\Antigravity IDE\resources\app\extensions\nova" -Recurse -Force
```

### Installation in VS Code / Cursor:
```powershell
Copy-Item -Path "editors\vscode" -Destination "$HOME\.vscode\extensions\nova-compiler.nova-language-0.1.0" -Recurse -Force
```

### Reloading the Editor:
1. Press `Ctrl + Shift + P`.
2. Select **`Developer: Reload Window`** (or restart the editor).
3. Any `.nova` file will automatically light up with syntax colors!

---

## 8. Keyword Counter Tool

A standalone Flex-based lexical tool [`tools/keyword_counter/counter.l`](tools/keyword_counter/counter.l) scans NOVA source files, counts occurrences of every reserved keyword, and outputs a summary.

```bash
# Run counter on demo.nova and generate keyword_counts.txt
make count-keywords
```

Output report ([`keyword_counts.txt`](keyword_counts.txt)):
```text
==================================================
           NOVA KEYWORD FREQUENCY REPORT          
==================================================
Source File   : examples/demo.nova
Unique Found  : 34 / 34 keywords
Total Occurred: 84 times
--------------------------------------------------
KEYWORD          COUNT
--------------------------------------------------
and              3
bool             2
case             2
char             2
const            4
cruise           1
default          1
detect           2
do               10
end              8
f32              4
fallback         1
false            2
flux             6
halt             1
i32              5
ignite           1
in               1
not              2
orbit            1
or               1
receive          1
redetect         1
require          2
ret              2
route            1
routine          2
skip             1
step             1
str              2
transmit         7
true             1
verify           2
void             1
==================================================
```

---

## 9. Language Quick Reference & Cheat Sheet

```nova
// Routine definition with preconditions
routine calculate_telemetry(distance: f32, time_delta: f32) -> f32 do
    require distance > 0.0 and time_delta > 0.0;
    ret distance / time_delta;
end

// Root execution capsule
ignite do
    // State declarations (primitives and arrays)
    const mission_name: str = "NOVA-VANGUARD";
    const channel: i32 = 42;
    const sensors: f32[3] = [12.4, 25.1, 40.0];
    flux signal_level: f32 = 1.0;
    flux online: bool = true;
    flux readings: i32[2] = [0, 0];

    readings[0] = 100;

    // Invariant verification contract
    verify online and signal_level > 0.0;

    // Counted stepped orbit cycle
    orbit cycle in 1..10 step 2 do
        detect cycle == 5 do
            skip; // skip iteration
        redetect cycle == 9 do
            halt; // break loop
        fallback do
            transmit("Cycle: ", cycle, " | Sensors: ", sensors);
        end
    end

    // Conditional cruise cycle
    cruise signal_level > 0.2 do
        signal_level = signal_level - 0.2;
    end

    // Signal route dispatch
    route channel do
        case 42:
            transmit("Telemetry locked.");
        default:
            transmit("Scanning...");
    end

    // Multi-argument Telemetry I/O
    transmit("Mission ", mission_name, " complete. Readings: ", readings);
end
```

---

## 10. Repository Structure

```text
NOVA-Compiler/
├── Makefile                      # Build automation (all, clean, test, count-keywords)
├── AGENTS.md                     # Non-negotiable engineering rules & phase workflow
├── README.md                     # Root project documentation (this file)
├── .gitignore                    # Build and IDE artifact exclusion
├── keyword_counts.txt            # Generated keyword frequency report
├── docs/                         # Authoritative project specifications
│   ├── language-spec.md          # Language syntax and semantics
│   ├── grammar.md                # Formal LALR(1) Bison grammar
│   ├── architecture.md           # Pipeline subsystem architecture
│   ├── testing.md                # Testing protocol and classification standards
│   └── research.md               # Contract & IR research specifications
├── src/                          # Compiler implementation (C++17)
│   ├── main.cpp                  # CLI entrypoint and driver
│   ├── lexer/                    # Lexical analysis subsystem
│   │   ├── token.hpp             # TokenKind enum and Token data structures
│   │   ├── scanner.l             # Flex lexical scanner specification
│   │   ├── lex.yy.c              # Generated Flex scanner
│   │   ├── lexer.hpp             # Lexer C++ class interface
│   │   └── lexer.cpp             # Lexer C++ driver
│   ├── parser/                   # Syntactic analysis subsystem (LALR(1))
│       ├── parser.y              # GNU Bison grammar specification
│       ├── parser.hpp            # Parser C++ class interface
│       └── parser.cpp            # Parser driver and token bridge
│   ├── ast/                      # Abstract Syntax Tree subsystem
│   │   ├── ast_node.hpp          # Polymorphic AST node hierarchy
│   │   ├── ast_visitor.hpp       # ASTVisitor interface
│   │   ├── ast_printer.hpp       # Indented AST dump visitor
│   │   └── ast_printer.cpp       # ASTPrinter implementation
│   ├── diagnostics/              # Source-location diagnostic engine
│   │   ├── diagnostic.hpp
│   │   └── diagnostic.cpp
│   └── semantic/                 # Semantic analysis & symbol table subsystem
│       ├── symbol.hpp            # Symbol, Scope, SymbolTable definitions
│       ├── symbol_table.cpp      # Lexical scope tree & offset tracking
│       ├── symbol_resolver.hpp   # Scope & mutability AST visitor
│       └── symbol_resolver.cpp   # SymbolResolver implementation
├── tests/                        # Automated test suites
│   ├── cli/                      # CLI driver verification tests
│   ├── lexer/                    # Comprehensive tokenization test cases
│   ├── parser/                   # Syntactic validity & diagnostic tests
│   ├── ast/                      # AST construction & dump tests
│   ├── expressions/              # Precedence, associativity & equation tests
│   └── semantic/                 # Scoping, redeclaration & mutability tests
├── examples/                     # Demonstration programs
│   └── demo.nova                 # Full demo exercising all 34 keywords
├── tools/                        # Utility programs
│   └── keyword_counter/          # Standalone Flex keyword counter tool
│       └── counter.l
├── editors/                      # Editor syntax highlighting integrations
│   └── vscode/                   # VS Code / Antigravity IDE extension package
│       ├── package.json
│       ├── language-configuration.json
│       └── syntaxes/
│           └── nova.tmLanguage.json
└── scripts/                      # Test automation scripts
    └── test_runner.py            # Automated test runner and classifier
```

---

## 11. Roadmap (Upcoming Phases)

- [x] **Phase 3: Bison Parser** — Core parser productions (`ignite do ... end`, `routine`, `T[N]`, multi-arg `transmit`), syntax recovery, and diagnostic source locations.
- [x] **Phase 4: Abstract Syntax Tree (AST)** — Polymorphic AST hierarchy (`ASTNode`, `ASTVisitor`, `--ast`).
- [x] **Phase 5: Expressions & Precedence** — Operator hierarchy and shift/reduce resolution.
- [x] **Phase 6: Variables & Symbol Table** — Scoping depth, `const` immutability, `flux` tracking.
- [ ] **Phase 7: Type Checking** — Static type enforcement for `i32`, `f32`, `bool`, `char`, `str`, and `T[N]` arrays.
- [ ] **Phase 8: Control Flow Semantics** — Branch and loop validation.
- [ ] **Phase 9: Routines (`routine`)** — Signature checking, arity verification, call validation.
- [ ] **Phase 10: I/O & Class Project Audit** — Multi-argument `transmit`, `receive`, feature matrix, and audit.
- [ ] **Phase 11: Class Project Release (`v1.0-lab`)** — Lab release milestone.
- [ ] **Phases 12–26: Research, TAC IR, Optimizations & LLVM Backend**.
