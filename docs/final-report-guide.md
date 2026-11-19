# NOVA Programming Language — Final Project Report Reference Guide

This guide provides the complete, structured technical explanation and verified code examples needed to write the **Handwritten Final Project Report** for the Compiler Design Laboratory submission.

---

## 1. Detailed Overview of the Designed Language, Features & Functionality

### 1.1 Language Identity & Purpose
**NOVA** is a statically typed, block-structured, imperative systems and telemetry programming language designed around a **Cosmic Mission Telemetry** domain vocabulary. Every program executes inside an explicit root capsule (`ignite do ... end`), preceded by zero or more modular procedural routines (`routine ... do ... end`).

### 1.2 Core Features & Functionality
1. **Strict Static Type System (No Implicit Coercions)**:
   - **Primitives**: `i32` (32-bit signed integer), `f32` (32-bit IEEE-754 float), `bool` (`true`/`false`), `char` (single byte character), `str` (UTF-8 string), and `void` (unit return type).
   - **Fixed-Size Homogeneous Arrays (`T[N]`)**: First-class compound array types (e.g., `i32[4]`, `f32[3]`) supporting array literals `[10, 20, 30, 40]`, zero-based indexing `arr[i]`, compile-time constant bounds verification (`0 <= i < N`), and runtime dynamic bounds checking.
2. **Explicit State Mutability Classification (`const` vs. `flux`)**:
   - `const`: Immutable binding requiring an initializer at declaration. Reassigning a `const` variable, mutating an element of a `const` array (`arr[0] = 1`), or reading input into a `const` variable via `receive` is rejected at compile time.
   - `flux`: Mutable dynamic binding with an optional initializer (defaulting to deterministic zero-values: `0`, `0.0`, `false`, `'\0'`, `""`, or zero-filled `T[N]`).
3. **Structured Control Flow Constructs**:
   - **Detection Branching**: `detect <cond> do ... redetect <cond> do ... fallback do ... end` (`if` / `else if` / `else`).
   - **Orbit Stepped Range Loops**: `orbit <idx> in <start>..<end> [step <s>] do ... end` where `<idx>` is implicitly scoped as an immutable `const i32`.
   - **Cruise Conditional Loops**: `cruise <cond> do ... end` (`while` loop).
   - **Route Multi-Way Dispatch**: `route <selector> do case <lit>: ... default: ... end` (`switch` over `i32`, `char`, or `bool` without fallthrough).
   - **Cycle Interruption**: `halt;` (`break`) and `skip;` (`continue`), strictly enforced inside enclosing loops.
4. **Routines & First-Class Design-by-Contract Verification**:
   - **Routines (`routine`)**: Support typed parameters (primitives and `T[N]` arrays), explicit return types (`-> <type>`), direct and mutual recursion, and compile-time all-paths return exhaustiveness checking.
   - **Preconditions (`require <bool>;`)**: Checked at the entry of a routine before executing its body.
   - **Runtime Invariants (`verify <bool>;`)**: Checked inline during execution; violations halt execution with exact `file:line:column` diagnostics.
5. **Telemetry I/O & Output File Generation**:
   - `transmit(e1, e2, ...);`: Multi-argument output statement capable of printing all primitive types and formatted arrays `[v0, v1, ..., vN-1]`.
   - `receive(flux_var);`: Reads standard input into a mutable `flux` variable.
   - **Output File Generation**: Running the NOVA compiler on an input file automatically writes the program's execution results to `output.txt` (or a custom path via `-o <file>`).

---

## 2. Complete Syntax Breakdown with Code Examples

### 2.1 Reserved Keywords (34 Total)

| Category | Keywords |
|---|---|
| **Program & Routines** | `ignite`, `routine`, `ret`, `do`, `end` |
| **State Mutability** | `const`, `flux` |
| **Data Types** | `i32`, `f32`, `bool`, `char`, `str`, `void` |
| **Branching & Routing** | `detect`, `redetect`, `fallback`, `route`, `case`, `default` |
| **Loops & Control** | `orbit`, `in`, `step`, `cruise`, `halt`, `skip` |
| **Telemetry I/O & Contracts** | `transmit`, `receive`, `verify`, `require` |
| **Logical & Boolean** | `and`, `or`, `not`, `true`, `false` |

### 2.2 State Declarations & Fixed-Size Arrays
```nova
const mission_tag: str = "NOVA-ORION";
const thresholds: f32[3] = [10.5, 20.0, 35.8];
flux counter: i32;                         // defaults to 0
flux readings: i32[4] = [10, 20, 30, 40];
readings[2] = 99;                          // valid: readings is flux
```

### 2.3 Expressions & 9-Level Operator Precedence Hierarchy

From highest precedence (Level 1) to lowest precedence (Level 9):

| Level | Operators | Associativity | Example |
|---|---|---|---|
| **1** | `()`, `fn(...)`, `arr[i]` | Left-to-right | `scale(arr[i + 1], (a + b))` |
| **2** | `**` (exponentiation) | Right-to-left | `2 ** 3 ** 2` $\rightarrow$ `2 ** (3 ** 2)` = `512` |
| **3** | `-` (unary), `not` | Right-to-left | `-a ** b` $\rightarrow$ `-(a ** b)` |
| **4** | `*`, `/`, `%` (`%` on `i32`) | Left-to-right | `24 / 6 / 2` $\rightarrow$ `(24 / 6) / 2` = `2` |
| **5** | `+`, `-` | Left-to-right | `10 - 3 - 2` $\rightarrow$ `(10 - 3) - 2` = `5` |
| **6** | `<`, `<=`, `>`, `>=` | Left-to-right | `altitude <= 120.0` |
| **7** | `==`, `!=` | Left-to-right | `cycle == 3` |
| **8** | `and` (short-circuit) | Left-to-right | `online and altitude > 0.0` |
| **9** | `or` (short-circuit) | Left-to-right | `flag1 or flag2` |

### 2.4 Control Flow Syntax Examples
```nova
// 1. Detection Branching
detect altitude > 500.0 do
    transmit("Deep space trajectory");
redetect altitude > 100.0 do
    transmit("Orbital trajectory");
fallback do
    transmit("Landing sequence");
end

// 2. Stepped Orbit Loop with Skip and Halt
orbit cycle in 1..10 step 1 do
    detect cycle == 3 do
        skip;
    redetect cycle == 7 do
        halt;
    fallback do
        transmit("Cycle: ", cycle);
    end
end

// 3. Conditional Cruise Loop
cruise altitude > 100.0 do
    altitude = altitude - 50.0;
end

// 4. Multi-Way Route Dispatch
route channel do
    case 101:
        transmit("Primary Channel");
    case 202:
        transmit("Backup Channel");
    default:
        transmit("Unknown Channel");
end
```

### 2.5 Routines, Recursion & Contracts
```nova
routine factorial(n: i32) -> i32 do
    require n >= 0;
    detect n <= 1 do
        ret 1;
    fallback do
        ret n * factorial(n - 1);
    end
end
```

---

## 3. Explanation of the Flex (`.l`) and Bison (`.y`) Implementations

### 3.1 Flex Lexical Scanner ([`src/lexer/scanner.l`](../src/lexer/scanner.l))
1. **Evolution from Standalone Lexer to Bison Token Provider**:
   - In the initial standalone lexical assignment ([`tools/keyword_counter/counter.l`](../tools/keyword_counter/counter.l)), Flex matched keywords and printed frequency tables directly.
   - In the compiler pipeline ([`src/lexer/scanner.l`](../src/lexer/scanner.l)), `YY_DECL` is defined so `yylex(nova::Token& out_token, const std::string& filename)` populates a structured `nova::Token` containing:
     - `TokenKind kind` (from [`src/lexer/token.hpp`](../src/lexer/token.hpp))
     - `std::string lexeme` (`yytext`)
     - `SourceLocation location` (`filename`, 1-indexed `line`, and 1-indexed `column`)
2. **Key Lexical Rules**:
   - **Comments**: Single-line `//[^\n]*` and multi-line `/* ... */` (via exclusive start condition `%x COMMENT` tracking newlines and reporting unterminated comments).
   - **Multi-Character Operators**: Checked before single-character operators (`**` before `*`, `<=` before `<`, `>=` before `>`, `==`, `!=`, `..`, `->`).
   - **Keywords vs. Identifiers**: All 34 reserved keywords are matched explicitly first; `[a-zA-Z_][a-zA-Z0-9_]*` matches general identifiers (`TokenKind::IDENT`).
   - **Literals**: Integer `[0-9]+`, floating-point `[0-9]+"."[0-9]+([eE][+-]?[0-9]+)?`, character `'([^'\\\n]|\\.)'`, and string `"([^"\\\n]|\\.)*"` literals (plus catch-all rules for malformed/unterminated literals and invalid characters).
3. **Bison Lexer Bridge (`nova_parser_lex` in [`src/parser/parser.cpp`](../src/parser/parser.cpp))**:
   - Calls `parser->consume_token_ref()`, binds `yylval_param->token = &tok`, populates Bison's `yylloc_param` (`first_line`, `first_column`, `last_line`, `last_column`), and returns the corresponding Bison terminal token ID.

### 3.2 Bison LALR(1) Parser ([`src/parser/parser.y`](../src/parser/parser.y))
1. **Pure Reentrant Configuration**:
   - Uses `%define api.pure full`, `%define api.prefix {nova_parser_}`, `%locations`, and `%define parse.error detailed`.
   - Compiles under `bison -Wall -Wcounterexamples` with **0 shift/reduce conflicts and 0 reduce/reduce conflicts**.
2. **Context-Free Grammar (CFG) Structure**:
   - `program`: Optional `routine_decl_list` followed by mandatory `ignite_block` (`KW_IGNITE KW_DO stmt_list KW_END`).
   - `routine_decl`: `KW_ROUTINE IDENT "(" opt_param_list ")" opt_return_type KW_DO opt_require_list stmt_list KW_END`.
   - `type`: Primitive `base_type` (`i32`, `f32`, `bool`, `char`, `str`, `void`) or fixed-size array `base_type "[" INT_LIT "]"`.
   - `stmt`: Declarations (`const`, `flux`), assignments (`id = expr` and `postfix_expr[expr] = expr`), control flow (`detect_stmt`, `cruise_stmt`, `orbit_stmt`, `route_stmt`, `halt`, `skip`, `ret`), contracts (`verify`), I/O (`transmit`, `receive`), expression statements, and panic-mode recovery (`error SEMICOLON { yyerrok; }`).
   - `expr` & `postfix_expr`: Explicit `%left`, `%right`, and `%precedence` declarations resolve all binary and unary operator precedence levels cleanly without grammar ambiguity, while `postfix_expr` handles `primary_expr`, array indexing `postfix_expr[expr]`, and `routine_call`.
3. **Semantic Actions & Memory-Safe Error Recovery**:
   - Every CFG reduction allocates the corresponding polymorphic C++ AST node (`ProgramNode`, `RoutineDeclNode`, `ConstDeclNode`, `FluxDeclNode`, `AssignNode`, `DetectNode`, `CruiseNode`, `OrbitNode`, `RouteNode`, `TransmitNode`, `ReceiveNode`, `VerifyNode`, `RetNode`, `BinaryExprNode`, `UnaryExprNode`, `IndexAccessNode`, `ArrayLiteralNode`, `RoutineCallNode`, `IdentifierNode`, `LiteralNode`).
   - `%destructor { delete $$; }` directives on all nonterminal semantic types guarantee zero memory leaks if a syntax error triggers stack unwinding.

---

## 4. How the Language Processes Code and Produces Output

### 4.1 End-to-End Processing Pipeline
When the user runs `./nova examples/submission_input.nova` (or `./nova examples/submission_input.nova -o custom_output.txt`):

1. **Stage 1 — Lexical Analysis (`Lexer` & Flex `scanner.l`)**:
   - Reads the `.nova` input file and converts raw characters into a vector of `Token` objects annotated with 1-indexed line and column numbers. Halts immediately if any lexical error is encountered.
2. **Stage 2 — Syntactic Analysis (`Parser` & Bison `parser.y`)**:
   - Feeds tokens into the LALR(1) parser (`nova_parser_parse`), validating grammar rules and constructing the rooted `ProgramNode` Abstract Syntax Tree (`AST`).
3. **Stage 3 — Lexical Scope & Mutability Resolution (`SymbolResolver`)**:
   - Traverses the AST to build the hierarchical `SymbolTable`, registering top-level routine signatures (enabling recursion), checking block scoping and variable shadowing, rejecting duplicate declarations in the same scope, and rejecting mutations of `const` scalars, `const` arrays, and `orbit` loop indices.
4. **Stage 4 — Static Type & Exhaustiveness Checking (`TypeChecker`)**:
   - Validates that every operator, initializer, assignment, array literal, array index (including compile-time constant bounds `0 <= idx < N`), branch condition (`bool`), `orbit` range/step (`i32`, non-zero step), `route` selector/case (`i32`/`char`/`bool` without duplicate cases), `halt`/`skip` loop context, and routine call/return (including all-paths return exhaustiveness) obeys NOVA's type rules.
5. **Stage 5 — AST Execution & Output File Generation (`Interpreter`)**:
   - Evaluates the verified AST starting at `ignite do ... end`, managing call frames for routine invocations, checking `require` preconditions and `verify` invariants at runtime, streaming `transmit(...)` output to standard output, and writing the complete execution output to **`output.txt`** (or the file specified via `-o`).

### 4.2 Verified Execution Example (`examples/submission_input.nova` $\rightarrow$ `output.txt`)
Command:
```bash
./nova examples/submission_input.nova
```
Generated `output.txt` (includes Program Execution Output, AST Construct Detection Summary for Functions/Loops/Branches/Declarations, Lexical Token Counts & 34/34 Keyword Frequencies, Lexical Symbol Table, and the Complete Token Stream):
```text
======================================================================
              NOVA COMPILER — ANALYSIS & EXECUTION REPORT             
======================================================================
Source File : examples/submission_input.nova

----------------------------------------------------------------------
1. PROGRAM EXECUTION OUTPUT (transmit Results)
----------------------------------------------------------------------
=== MISSION: NOVA-ORION-V1 | SECTOR: G ===
[FUNCTION DETECTED #4] log_banner(mission: str, sector: char) -> void
[FUNCTION DETECTED #3] sum_array(i32[4]) -> i32 | [LOOP DETECTED #1: orbit] iterations = 4
[FUNCTION DETECTED #1] calibrate_gain(sensor_id=1, base_gain=1.5, factor=2.0) -> f32
[FUNCTION DETECTED #2] factorial(5) -> i32 (recursive calls = 5, result = 120)
Telemetry Samples : [10, 20, 35, 40] | Sum = 105
Calibrated Gain   : 3.25 | Weights = [1.5, 2.0, 4.25]
Factorial(5)      : 120 | Precedence Check = 492
[LOOP DETECTED #2: orbit 1..10 step 1] iterations = 7, skip = 1, halt = 1, accum = 18
[LOOP DETECTED #3: cruise] iterations = 3, final altitude = 120.0 m
[BRANCH DETECTED: route case 202] BACKUP-UHF-RELAY
[DETECTION SUMMARY] Functions Defined = 4 | Loops Executed = 3 | Total Loop Iterations = 14
=== TELEMETRY SEQUENCE COMPLETE ===

----------------------------------------------------------------------
2. SYNTACTIC & AST CONSTRUCT DETECTION SUMMARY
----------------------------------------------------------------------
Functions / Routines Detected : 4 (Total Calls: 5)
  [Function #1] routine calibrate_gain(sensor_id: i32, base_gain: f32, factor: f32) -> f32 (line 13, col 1)
  [Function #2] routine factorial(n: i32) -> i32 (line 19, col 1)
  [Function #3] routine sum_array(samples: i32[4]) -> i32 (line 28, col 1)
  [Function #4] routine log_banner(mission: str, sector: char) -> void (line 39, col 1)
Loops Detected                : 3 (orbit: 2, cruise: 1, skip: 1, halt: 1)
Conditional Branches Detected : 5 (detect: 4, redetect: 1, fallback: 2, route: 1, case: 2, default: 1)
State Declarations Detected   : 22 (const: 8, flux: 14, arrays T[N]: 3, assignments: 12)
Contracts & I/O Detected      : require: 3, verify: 2, transmit: 15, receive: 1

----------------------------------------------------------------------
3. LEXICAL TOKEN COUNTS & KEYWORD FREQUENCY
----------------------------------------------------------------------
Total Tokens Scanned          : 656
  - Keywords                  : 144 (34 unique)
  - Identifiers               : 116
  - Literals                  : 101
  - Operators                 : 75
  - Punctuators / Delimiters  : 220
...
```
