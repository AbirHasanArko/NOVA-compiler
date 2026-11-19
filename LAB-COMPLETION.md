# NOVA Compiler — Mandatory Class Project Completion Report (`v1.0-lab`)

## 1. Required Course Deliverables Summary

| Required Deliverable | Repository File / Artifact | Description & Verification Command |
|---|---|---|
| **1. Handwritten Final Project Report Guide** | [`docs/final-report-guide.md`](docs/final-report-guide.md) | Complete reference material covering language overview, syntax breakdown with examples, Flex/Bison implementation details, and end-to-end execution/output pipeline. |
| **2. Flex Lexical Specification (`.l`)** | [`src/lexer/scanner.l`](src/lexer/scanner.l)<br>(and standalone [`tools/keyword_counter/counter.l`](tools/keyword_counter/counter.l)) | Flex scanner returning typed tokens with 1-indexed `line:col` locations to Bison (`./nova --tokens examples/submission_input.nova`). |
| **3. Bison Grammar Specification (`.y`)** | [`src/parser/parser.y`](src/parser/parser.y) | Pure reentrant LALR(1) Bison grammar (`0` shift/reduce and `0` reduce/reduce conflicts) constructing the polymorphic AST (`./nova --ast examples/submission_input.nova`). |
| **4. Input File & Output File Generation** | [`examples/submission_input.nova`](examples/submission_input.nova)<br>[`examples/demo.nova`](examples/demo.nova)<br>$\rightarrow$ generates `output.txt` | Running `./nova examples/submission_input.nova` processes the NOVA source program and generates `output.txt` (or `-o <file>`) with the corresponding execution results. |

---

## 2. Quick-Start Verification for Evaluators

```bash
# 1. Clean build of Flex scanner, Bison parser, semantic passes, and AST interpreter
make clean && make

# 2. Run the submission input file and inspect the generated output.txt
./nova examples/submission_input.nova
cat output.txt

# 3. Inspect intermediate compiler pipeline stages
./nova --tokens examples/submission_input.nova
./nova --parse examples/submission_input.nova
./nova --ast examples/submission_input.nova
./nova --symbols examples/submission_input.nova

# 4. Run the full 5-state automated regression test suite
make test
```

---

## 3. Completed Engineering Phases (Phases 0–11)

- **Phase 0 — Specification & Architecture**: Formal 34-keyword language specification ([`docs/language-spec.md`](docs/language-spec.md)), LALR(1) grammar ([`docs/grammar.md`](docs/grammar.md)), modular architecture ([`docs/architecture.md`](docs/architecture.md)), and testing protocol ([`docs/testing.md`](docs/testing.md)).
- **Phase 1 — Project Bootstrap & CLI Harness**: Cross-platform `Makefile`, CLI driver ([`src/main.cpp`](src/main.cpp)), and 5-state Python test harness ([`scripts/test_runner.py`](scripts/test_runner.py)).
- **Phase 2 — Flex Lexer & Tooling**: [`src/lexer/scanner.l`](src/lexer/scanner.l), token stream driver, standalone keyword counter ([`tools/keyword_counter/counter.l`](tools/keyword_counter/counter.l)), VS Code/Antigravity syntax extension ([`editors/vscode/`](editors/vscode/)), and lexer test suite.
- **Phase 3 — Bison LALR(1) Parser**: [`src/parser/parser.y`](src/parser/parser.y) with `0` conflicts, statement-level error recovery, `--parse` CLI flag, and parser test suite.
- **Phase 4 — Abstract Syntax Tree (AST)**: 24-node polymorphic AST hierarchy ([`src/ast/ast_node.hpp`](src/ast/ast_node.hpp)), `ASTVisitor`, `ASTPrinter`, `--ast` CLI flag, and AST test suite (`v0.1-parser-ast`).
- **Phase 5 — Expressions & Precedence**: Verified 9-level operator precedence and associativity (`**`, unary `-`/`not`, `*`/`/`/`%`, `+`/`-`, relational, equality, `and`, `or`), array indexing, and parenthesized equations.
- **Phase 6 — Variables & Symbol Table**: Source-location [`DiagnosticEngine`](src/diagnostics/diagnostic.hpp), hierarchical [`SymbolTable`](src/semantic/symbol.hpp), [`SymbolResolver`](src/semantic/symbol_resolver.cpp) enforcing `const` vs `flux` mutability and lexical scoping, and `--symbols` CLI flag.
- **Phase 7 — Type Checking**: Standalone [`TypeChecker`](src/semantic/type_checker.cpp) enforcing strict static typing across primitives and `T[N]` arrays, compile-time constant array bounds checking, and operator/condition type rules.
- **Phase 8 — Control Flow Semantics**: Validation of `detect`/`redetect`/`fallback`, `cruise`, `orbit` (including zero-step rejection), `route`/`case`/`default` (including duplicate case detection), and loop-context enforcement for `halt` and `skip`.
- **Phase 9 — Routines (`routine`)**: Routine signature registration, direct & mutual recursion, array parameters/returns, call arity/type validation, `require` preconditions, and all-paths `ret` exhaustiveness analysis.
- **Phase 10 — Telemetry I/O, Execution & Output Generation**: AST execution engine ([`src/codegen/interpreter.cpp`](src/codegen/interpreter.cpp)), multi-argument `transmit`, `receive`, runtime `verify`/`require` contracts, automatic `output.txt` / `-o <file>` output generation, and feature traceability matrix ([`docs/class-feature-matrix.md`](docs/class-feature-matrix.md)).
- **Phase 11 — Mandatory Class Project Release (`v1.0-lab`)**: Deterministic submission input [`examples/submission_input.nova`](examples/submission_input.nova), handwritten report reference guide [`docs/final-report-guide.md`](docs/final-report-guide.md), and full release verification.
