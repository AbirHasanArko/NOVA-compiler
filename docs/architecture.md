# NOVA Compiler Architecture

This document describes the internal architectural design, subsystem interfaces, and dataflow of the **NOVA Compiler** (`nova`).

---

## 1. High-Level Compiler Pipeline

```text
       +-----------------------+
       |   Source Code (.nova) |
       +-----------------------+
                   |
                   v
       +-----------------------+
       |  Lexer (Flex/Scanner) |  <-- Token Stream + Line/Col Coordinates
       +-----------------------+
                   |
                   v
       +-----------------------+
       |  Parser (Bison/C++)   |  <-- Syntax Trees / Syntactic Errors
       +-----------------------+
                   |
                   v
       +-----------------------+
       | Abstract Syntax Tree  |  <-- Canonical AST Representation
       +-----------------------+
                   |
                   v
       +-----------------------+
       | Scope & Symbol Table  |  <-- Symbol Resolution & Mutability Tracking
       +-----------------------+
                   |
                   v
       +-----------------------+
       |     Type Checker      |  <-- Static Type Verification
       +-----------------------+
                   |
         +---------+---------+
         |                   |
         v                   v
+-----------------+ +-------------------+
| Baseline IR Pass| | Contract Analyzer |
+-----------------+ +-------------------+
         |                   |
         |          +--------+--------+
         |          | Contract-Aware  |
         |          | Intermediate    |
         |          | Rep (TAC/IR)    |
         |          +--------+--------+
         |                   |
         |                   v
         |          +-------------------+
         |          | Contract-Guided   |
         |          | Optimization Pass |
         |          +-------------------+
         |                   |
         +---------+---------+
                   |
                   v
       +-----------------------+
       | Target / Differential |
       |     Execution / VM    |
       +-----------------------+
```

---

## 2. Subsystem Descriptions

### 2.1 Diagnostics Subsystem (`src/diagnostics/`)
- **`SourceLocation`**: Encapsulates filename, 1-indexed line, and 1-indexed column.
- **`DiagnosticEngine`**: Standardizes error, warning, and note formatting:
  ```text
  tests/semantic/invalid_assign.nova:12:5: error: cannot assign to immutable variable 'x'
      x = 42;
      ^
  ```
- Collects diagnostics and controls whether subsequent compiler stages abort.

### 2.2 Lexical Subsystem (`src/lexer/`)
- Implemented via Flex (`scanner.l`) compiled to modern C++.
- Encapsulates state within a scanner class or reentrant interface.
- Tracks exact line and column numbers using custom actions.
- Outputs `Token` structures containing token kind, semantic value (`std::string`, `int32_t`, etc.), and `SourceLocation`.

### 2.3 Syntactic Subsystem (`src/parser/`)
- Implemented via Bison (`parser.y`) generating a clean LALR(1) parser.
- Pure parser action: directly constructs polymorphically typed AST nodes.
- Integrates with the DiagnosticEngine for syntax error recovery and precise diagnostic reporting.

### 2.4 Abstract Syntax Tree (`src/ast/`)
- Abstract base class `ASTNode` with `SourceLocation`.
- Hierarchical categories:
  - `ProgramNode`: Root containing functions and entry block.
  - `StatementNode`: `VarDeclNode`, `ValDeclNode`, `AssignNode`, `IfNode`, `WhileNode`, `LoopNode`, `SelectNode`, `EmitNode`, `ReadNode`, `AssertNode`, `RetNode`, etc.
  - `ExpressionNode`: `BinaryExprNode`, `UnaryExprNode`, `LiteralNode`, `IdentifierNode`, `CallExprNode`.
- Implements the **Visitor Pattern** (`ASTVisitor`) to decouple AST representation from semantic analysis, printing, and IR emission passes.

### 2.5 Semantic Analysis Subsystem (`src/semantic/`)
- **`Symbol`**: Stores identifier name, `Type`, mutability (`VAL` vs `VAR`), declaration `SourceLocation`, and memory offset/register id.
- **`Scope`**: Lexical tree supporting lookup in current scope and parent scopes.
- **`SymbolTable`**: Manages scope entry and exit, handles shadowing correctly, and rejects duplicate identifiers at the same scope depth.
- **`TypeChecker`**: AST Visitor that inspects expression types, validates operator operand compatibility, ensures non-void functions return a matching value along all control paths, and validates loop bounds.

### 2.6 Contract System (`src/contracts/`)
- Encapsulates preconditions (`require`) and runtime assertions (`assert`).
- **`ContractAnalyzer`**: Associates preconditions with function signatures and tracks invariants through sequential basic blocks.

### 2.7 Intermediate Representation Subsystem (`src/ir/`)
- Three-Address Code (TAC) Quadruple representation:
  - `Quad`: Opcode, Argument 1, Argument 2, Result Destination.
  - Op codes: `ADD`, `SUB`, `MUL`, `DIV`, `MOD`, `POW`, `CMP_LT`, `CMP_EQ`, `JUMP`, `JUMP_IF_FALSE`, `CALL`, `PARAM`, `RETURN`, `EMIT`, `READ`, `ASSERT`, `CONTRACT`.
- **`BasicBlock`**: Sequence of straight-line instructions ending with a branch or terminator.
- **`ControlFlowGraph (CFG)`**: Graph of basic blocks per function and the `entry` routine.

### 2.8 Optimization Subsystem (`src/optimization/`)
- **`ConstantPropagation`**: Propagates known scalar values through TAC basic blocks.
- **`ContractGuidedDCE`**: Uses contract facts and invariants to evaluate branch conditions at compile time, eliminating dead branches safely.
- **Safety guarantee**: Invalidation logic resets inferred facts upon variable mutation.

---

## 3. Command-Line Interface (`nova`)

The compiler driver (`src/main.cpp`) exposes progressive pipeline inspection flags:

| Flag | Description |
|---|---|
| `--help` | Display usage instructions and supported flags |
| `--tokens <file>` | Lex the file and print the token stream with locations |
| `--ast <file>` | Parse the file and print the AST in indented tree format |
| `--symbols <file>` | Perform semantic analysis and dump the symbol tables |
| `--contracts <file>` | Inspect active contracts and function preconditions |
| `--baseline-ir <file>` | Emit baseline TAC without contract optimizations |
| `--ir <file>` | Emit contract-aware and optimized TAC |
| `-o <output>` | Specify output target |
