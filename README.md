# NOVA Programming Language & Compiler

> **NOVA**: A compiled, statically-typed programming language engineered with a **Cosmic Telemetry & Systems Computing** aesthetic. Built with Flex, Bison, and C++17.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](#)
[![Phase](https://img.shields.io/badge/phase-Phase%202%20Complete-blue)](#)
[![License](https://img.shields.io/badge/license-MIT-informational)](#)

---

## Table of Contents
- [1. Overview & Linguistic Philosophy](#1-overview--linguistic-philosophy)
- [2. Project Status & Completed Milestones](#2-project-status--completed-milestones)
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
- **Procedures (`proc ... ret`)**: High-efficiency routines with return types and preconditions.
- **State Classification (`const` vs `flux`)**:
  - `const`: Immutable state; mandatory initializer; cannot be reassigned.
  - `flux`: Dynamic mutable state; allows runtime mutations.
- **Execution Cycles**:
  - `orbit <idx> in <start>..<end> [step <s>] do ... end`: Counted stepped range loop (`for`).
  - `cruise <condition> do ... end`: Telemetry condition loop (`while`).
  - `halt;` and `skip;`: Loop interruption (`break` and `continue`).
- **Detection Branching (`detect` / `redetect` / `fallback`)**:
  - `detect`: Initial condition check (`if`).
  - `redetect`: Subsequent branch check (`elif`).
  - `fallback`: Contingency fallback block (`else`).
- **Telemetry I/O**:
  - `transmit(<expr>);`: Standard output stream.
  - `receive(<flux_var>);`: Standard input scanner.
- **Contracts & Safety**:
  - `verify <cond>;`: Runtime invariant assertion.
  - `require <cond>;`: Procedure precondition validation.

---

## 2. Project Status & Completed Milestones

| Milestone | Status | Description |
|---|---|---|
| **Phase 0: Specification & Setup** |  Completed | Authored authoritative language spec (`docs/language-spec.md`), LALR(1) grammar (`docs/grammar.md`), compiler architecture (`docs/architecture.md`), testing protocol (`docs/testing.md`), and agent rules (`AGENTS.md`). |
| **Phase 1: Project Bootstrap** |  Completed | Built cross-platform `Makefile`, core driver (`src/main.cpp`), CLI parser (`--help`, `--version`), and 5-state test harness (`scripts/test_runner.py`). |
| **Phase 2: Flex Lexer** |  Completed | Implemented complete token model (`src/lexer/token.hpp`), Flex scanner (`src/lexer/scanner.l` -> `src/lexer/lex.yy.c`), C++ driver (`src/lexer/lexer.cpp`), `--tokens` CLI inspection flag, and test suite. |
| **Demo Program** |  Completed | Comprehensive demonstration program (`examples/demo.nova`) exercising all 34 reserved keywords. |
| **Standalone Keyword Tool** |  Completed | Standalone Flex analyzer (`tools/keyword_counter/counter.l`) generating keyword frequency reports (`keyword_counts.txt`). |
| **Editor Syntax Highlighting** |  Completed | TextMate grammar (`editors/vscode/syntaxes/nova.tmLanguage.json`) and VS Code / Antigravity IDE native extension package. |
| **Phase 3: Bison Parser** | ⏳ Next | LALR(1) syntax analysis and syntax error diagnostics. |

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

Build operations are managed through the root [`Makefile`](file:///d:/Documents/NOVA-Compiler/Makefile):

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
  --ast <file>            Syntax analysis: parse and dump AST (Phase 4)
  --symbols <file>        Semantic analysis: dump symbol tables (Phase 6)
  --contracts <file>      Verification: inspect contracts & preconditions (Phase 15)
  --baseline-ir <file>    IR: emit baseline Three-Address Code (Phase 17)
  --ir <file>             IR: emit contract-aware optimized TAC (Phase 16)
  -o <file>               Specify output artifact destination
```

### Example Commands:
```bash
# Inspect token stream of a NOVA program
./nova --tokens examples/demo.nova

# Scan a lexer test file
./nova --tokens tests/lexer/keywords.nova
```

---

## 6. Automated Testing Suite

The test harness [`scripts/test_runner.py`](file:///d:/Documents/NOVA-Compiler/scripts/test_runner.py) strictly validates test outputs against five mutually exclusive statuses:

- `PASS`: Valid test executed and output matched expectations.
- `EXPECTED ERROR`: Negative test produced the expected diagnostic message and error code.
- `UNEXPECTED ERROR`: Valid test failed, or invalid test failed with the wrong error code/message.
- `FAIL`: Output or exit code mismatch.
- `CRASH`: Compiler segfaulted, aborted, or crashed.

### Running the Suite:
```bash
make test
```

Current test coverage: **15 / 15 tests passing** across CLI flags, keywords, operators, literals, identifiers, comments, coordinates, and lexical error diagnostics.

---

## 7. Editor Syntax Highlighting Extension

A custom TextMate syntax highlighting package is included in [`editors/vscode/`](file:///d:/Documents/NOVA-Compiler/editors/vscode/):
- **Grammar**: [`editors/vscode/syntaxes/nova.tmLanguage.json`](file:///d:/Documents/NOVA-Compiler/editors/vscode/syntaxes/nova.tmLanguage.json)
- **Language Config**: [`editors/vscode/language-configuration.json`](file:///d:/Documents/NOVA-Compiler/editors/vscode/language-configuration.json) (bracket matching, auto-closing quotes, `do ... end` block indentation)
- **Manifest**: [`editors/vscode/package.json`](file:///d:/Documents/NOVA-Compiler/editors/vscode/package.json)

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

A standalone Flex-based lexical tool [`tools/keyword_counter/counter.l`](file:///d:/Documents/NOVA-Compiler/tools/keyword_counter/counter.l) scans NOVA source files, counts occurrences of every reserved keyword, and outputs a summary.

```bash
# Run counter on demo.nova and generate keyword_counts.txt
make count-keywords
```

Output report ([`keyword_counts.txt`](file:///d:/Documents/NOVA-Compiler/keyword_counts.txt)):
```text
==================================================
           NOVA KEYWORD FREQUENCY REPORT          
==================================================
Source File   : examples/demo.nova
Unique Found  : 34 / 34 keywords
Total Occurred: 82 times
--------------------------------------------------
KEYWORD          COUNT
--------------------------------------------------
and              3
bool             2
case             2
char             2
const            3
cruise           1
default          1
detect           2
do               10
end              8
f32              3
fallback         1
false            2
flux             5
halt             1
i32              4
ignite           1
in               1
not              2
orbit            1
or               1
proc             2
receive          1
redetect         1
require          2
ret              2
route            1
skip             1
step             1
str              2
transmit         9
true             1
verify           2
void             1
==================================================
```

---

## 9. Language Quick Reference & Cheat Sheet

```nova
// Procedure definition with preconditions
proc calculate_telemetry(distance: f32, time_delta: f32) -> f32 do
    require distance > 0.0 and time_delta > 0.0;
    ret distance / time_delta;
end

// Root execution capsule
ignite do
    // State declarations
    const mission_name: str = "NOVA-VANGUARD";
    const channel: i32 = 42;
    flux signal_level: f32 = 1.0;
    flux online: bool = true;

    // Invariant verification contract
    verify online and signal_level > 0.0;

    // Counted stepped orbit cycle
    orbit cycle in 1..10 step 2 do
        detect cycle == 5 do
            skip; // skip iteration
        redetect cycle == 9 do
            halt; // break loop
        fallback do
            transmit(cycle);
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

    // Telemetry I/O
    transmit("Mission complete.");
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
│   └── testing.md                # Testing protocol and classification standards
├── src/                          # Compiler implementation (C++17)
│   ├── main.cpp                  # CLI entrypoint and driver
│   └── lexer/                    # Lexical analysis subsystem
│       ├── token.hpp             # TokenKind enum and Token data structures
│       ├── scanner.l             # Flex lexical scanner specification
│       ├── lex.yy.c              # Generated Flex scanner
│       ├── lexer.hpp             # Lexer C++ class interface
│       └── lexer.cpp             # Lexer C++ driver
├── tests/                        # Automated test suites
│   ├── cli/                      # CLI driver verification tests
│   └── lexer/                    # Comprehensive tokenization test cases
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

- [ ] **Phase 3: Bison Parser** — Core parser productions (`ignite do ... end`), syntax recovery, and diagnostic source locations.
- [ ] **Phase 4: Abstract Syntax Tree (AST)** — Polymorphic AST hierarchy (`ASTNode`, `ASTVisitor`, `--ast`).
- [ ] **Phase 5: Expressions & Precedence** — Operator hierarchy and shift/reduce resolution.
- [ ] **Phase 6: Variables & Symbol Table** — Scoping depth, `const` immutability, `flux` tracking.
- [ ] **Phase 7: Type Checking** — Static type enforcement for `i32`, `f32`, `bool`, `char`, `str`.
- [ ] **Phase 8: Control Flow Semantics** — Branch and loop validation.
- [ ] **Phase 9: Procedures** — Signature checking, arity verification, call validation.
- [ ] **Phase 10: I/O & Class Project Audit** — Feature matrix and audit.
- [ ] **Phase 11: Class Project Release (`v1.0-lab`)** — Lab release milestone.
- [ ] **Phases 12–26: Research, TAC IR, Optimizations & LLVM Backend**.
