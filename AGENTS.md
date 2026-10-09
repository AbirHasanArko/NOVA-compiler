# AGENTS.md — Non-Negotiable Engineering Rules & Workflow

This document defines the mandatory guidelines, constraints, and protocols for any human developer or AI coding agent working on the **NOVA Compiler** project.

---

## 1. Core Principles

1. **Authoritative Specification**: `docs/language-spec.md` is the ground truth. Never invent, modify, or extend syntax or semantics without explicit prior user approval.
2. **Grammar Consistency**: Before coding or modifying grammar rules, ensure the grammar is strictly unambiguous and implementable in Flex (lexer) and Bison (LALR(1) parser).
3. **No Phantom Passes**: Never claim a test passed unless it was genuinely executed via shell command and the output verified. Never declare a phase complete merely because source files compile.
4. **Zero Regressions**: Run all existing regression tests whenever a new feature or fix is introduced. Regressions must be resolved before proceeding.
5. **Separation of Concerns**: Keep components strictly decoupled. Lexer, Parser, AST, Symbol Table, Semantic Analysis, IR, and Optimization must remain in separate modules under `src/`.
6. **No Premature Dependencies**: Do not introduce CMake, LLVM, external parsers, or complex third-party libraries unless explicitly mandated by the approved plan.
7. **Small, Atomic Commits**: Commit logically grouped changes with standard conventional commit prefixes (`feat:`, `fix:`, `test:`, `docs:`, `refactor:`).
8. **Semantic Preservation**: Every optimization stage must provably preserve program semantics.

---

## 2. Directory Layout & Module Responsibilities

```text
nova-compiler/
├── Makefile                # Build automation (make, make clean, make test)
├── AGENTS.md               # Compiler guidelines and agent rules (this file)
├── README.md               # Project overview and instructions
├── docs/                   # Authoritative documentation
│   ├── language-spec.md    # Formal syntax and semantics specification
│   ├── grammar.md          # Formal BNF/EBNF grammar
│   ├── architecture.md     # Pipeline and module design
│   ├── testing.md          # Test protocol and conventions
│   ├── research.md         # Contract & IR research specifications
│   └── class-feature-matrix.md # Feature-to-test mapping matrix
├── src/                    # Compiler implementation (C++17)
│   ├── lexer/              # Flex scanner (.l)
│   ├── parser/             # Bison grammar (.y)
│   ├── ast/                # Abstract Syntax Tree nodes & visitors
│   ├── semantic/           # Symbol table, scope resolver, type checker
│   ├── contracts/          # Contract extraction & invariant analysis
│   ├── ir/                 # Intermediate representation (TAC / Quadruples)
│   ├── optimization/       # Dataflow passes, contract propagation, DCE
│   ├── codegen/            # Target code generation
│   ├── diagnostics/        # Source-location aware diagnostic engine
│   └── main.cpp            # Compiler CLI driver
├── tests/                  # Structured test suites
│   ├── lexer/              # Token and lexer diagnostics tests
│   ├── parser/             # Syntactic validity and syntax error tests
│   ├── ast/                # AST structure and dump tests
│   ├── semantic/           # Type check, scoping, and mutability tests
│   ├── control_flow/       # if/while/loop/select integration tests
│   ├── functions/          # Function call, recursion, signature tests
│   ├── contracts/          # Runtime assertions and preconditions tests
│   ├── ir/                 # IR generation and verification tests
│   ├── optimization/       # Optimization validity & adversary tests
│   └── integration/        # End-to-end NOVA programs
└── scripts/                # Test runner and differential test utilities
```

---

## 3. Phase Development Workflow

For every phase:
1. **Analyze**: Review phase objectives and relevant documents in `docs/`.
2. **Plan**: Formulate the minimal incremental design.
3. **Implement**: Code the changes cleanly in `src/`.
4. **Test Suite Addition**: Add minimal valid, normal valid, boundary, and negative/invalid test cases in `tests/`.
5. **Execute**: Build the project and run test suites using `make test`.
6. **Regression Verification**: Ensure all pre-existing tests continue to pass.
7. **Report**: Produce the standardized verification report before marking the phase complete.

### Standard Phase Verification Report Format

```text
PHASE: <Phase Number and Title>
IMPLEMENTED: <Brief summary of what was built>
FILES CREATED/MODIFIED: <List of modified or new files>
TEST FILES CREATED: <List of new test cases>
COMMANDS EXECUTED: <Exact commands run>
BUILD STATUS: PASS / FAIL
NEW TEST RESULTS: <Pass count, expected failure count>
REGRESSION RESULTS: <All prior tests passing count>
KNOWN LIMITATIONS: <Any deferred items or known bounds>
MANUAL TESTING INSTRUCTIONS: <CLI commands to reproduce>
```

---

## 4. Testing Rules

- A negative test (syntax error or semantic rejection) passes **only** if it fails with the expected diagnostic message and code, not due to a segmentation fault or unhandled exception.
- Tests must distinguish:
  - `PASS`: Output matches expected valid output.
  - `EXPECTED ERROR`: Expected syntax or type diagnostic was raised cleanly.
  - `UNEXPECTED ERROR`: Test succeeded when it should fail, or failed for the wrong reason.
  - `FAIL`: Output mismatch or unexpected exit status.
  - `CRASH`: Compiler segfaulted, aborted, or crashed.

---

## 5. Priority Hierarchy

1. **Priority 1 (Mandatory Lab Release)**: Flex Lexer, Bison Parser, AST, Symbol Table, Scopes, Type Checker, Control Flow, Functions, I/O, Error Diagnostics.
2. **Priority 2 (Compiler Engineering)**: Clean TAC IR generation, test runner scripts, diagnostics reporting.
3. **Priority 3 (Research Foundations)**: Runtime contracts, Preconditions, Contract IR representation.
4. **Priority 4 (Advanced Research)**: Contract-guided optimization, Constant propagation, Invalidation, Differential testing.

*Safety boundary: Never break Priority 1 features to achieve research goals.*
