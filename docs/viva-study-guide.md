# NOVA Compiler — Complete Study, Demo & Viva Defense Guide

This guide covers everything you need to know for your project presentation, live demonstration, and oral examination (viva): how every file works, how to compile manually or with `make`, every CLI command, how to justify the architecture and extra engineering work, and answers to likely instructor questions.

---

## 1. The Big Picture: How NOVA Works (60-Second Elevator Pitch)

> **"NOVA is a statically typed, block-structured programming language designed with a Cosmic Mission Telemetry theme. Instead of evaluating expressions directly inside Bison grammar actions—which breaks on loops, functions, and recursion—our compiler follows a true multi-pass textbook compiler pipeline:
> 1. **Flex (`scanner.l`)** tokenizes source text into tokens with 1-indexed `line:column` coordinates.
> 2. **Bison (`parser.y`)** parses the token stream using an unambiguous LALR(1) grammar (`0` shift/reduce and `0` reduce/reduce conflicts) and builds an in-memory **Abstract Syntax Tree (AST)**.
> 3. **Semantic Passes (`SymbolResolver` & `TypeChecker`)** walk the AST using the Visitor Pattern to verify lexical scoping, `const` vs. `flux` mutability, static types (`i32`, `f32`, `bool`, `char`, `str`, `T[N]` arrays), array bounds, and return path exhaustiveness.
> 4. **Execution Pass (`Interpreter`)** executes the verified AST starting at `ignite do ... end`, evaluates routines and recursion, checks `require` and `verify` contracts, prints `transmit(...)` results to the console, and automatically writes the results to `output.txt`."**

---

## 2. How to Compile Each File & Run Commands

### 2.1 Important Note on Windows (MSYS2 PATH)
If you open a fresh PowerShell terminal and `make`, `flex`, `bison`, or `g++` says *"command not found"*, run this one line first to add MSYS2 to your current PowerShell session:
```powershell
$env:PATH = "C:\msys64\usr\bin;C:\msys64\ucrt64\bin;" + $env:PATH
```

### 2.2 Automated Build Commands (Using `Makefile`)

| Goal | Command | What It Does |
|---|---|---|
| **Clean & Full Build** | `make clean && make` | Deletes old build artifacts, runs `flex` on `scanner.l`, runs `bison` on `parser.y`, compiles all C++17 modules, and links `nova.exe`. |
| **Build Only Changed Files** | `make` | Incrementally recompiles any modified `.l`, `.y`, or `.cpp` file. |
| **Run All 56 Automated Tests** | `make test` | Runs `python scripts/test_runner.py` across all 56 positive and negative test cases. |
| **Run Standalone Keyword Counter** | `make count-keywords` | Compiles `tools/keyword_counter/counter.l` and counts all 34 keywords in `examples/demo.nova` $\rightarrow$ `keyword_counts.txt`. |

---

### 2.3 Manual Step-by-Step Compilation (If a Teacher Asks How Flex/Bison/G++ Compile Under the Hood)

If the instructor asks: *"How do you compile the `.l` and `.y` files without `make`?"*, here are the exact 4 terminal commands that `make` executes:

```bash
# Step 1: Run Flex on scanner.l to generate the C scanner (src/lexer/lex.yy.c)
flex -o src/lexer/lex.yy.c src/lexer/scanner.l

# Step 2: Run Bison on parser.y to generate the LALR(1) parser C++ source & token header
# (-Wall enables all grammar warnings; -d generates src/parser/parser.tab.hpp)
bison -Wall -d -o src/parser/parser.tab.cpp src/parser/parser.y

# Step 3: Compile and link all sources with g++ (C++17) into nova.exe
g++ -std=c++17 -Wall -Wextra -pedantic -Isrc \
    src/main.cpp \
    src/lexer/lexer.cpp \
    -x c++ src/lexer/lex.yy.c \
    src/parser/parser.cpp \
    src/parser/parser.tab.cpp \
    src/ast/ast_printer.cpp \
    src/diagnostics/diagnostic.cpp \
    src/semantic/symbol_table.cpp \
    src/semantic/symbol_resolver.cpp \
    src/semantic/type_checker.cpp \
    src/codegen/interpreter.cpp \
    -o nova.exe
```
*(Note: `-x c++ src/lexer/lex.yy.c` tells `g++` to compile Flex's generated `.c` file as C++17 so it can directly fill C++ `nova::Token` objects!)*

**How to manually compile the standalone Lab 1 Keyword Counter (`counter.l`):**
```bash
flex -o build/counter.yy.c tools/keyword_counter/counter.l
g++ -std=c++17 -o bin/keyword_counter.exe build/counter.yy.c
./bin/keyword_counter.exe examples/demo.nova keyword_counts.txt
```

---

### 2.4 Every Available `./nova.exe` CLI Command & How to Generate Output

```bash
# 1. DEFAULT COMPILE & EXECUTE (Processes input file, prints results, and generates output.txt)
./nova.exe examples/submission_input.nova
cat output.txt

# 2. CUSTOM OUTPUT FILE (-o <filename>)
./nova.exe examples/submission_input.nova -o my_results.txt
cat my_results.txt

# 3. DIRECT INTERPRET MODE (Prints only program output to console + writes output.txt / -o)
./nova.exe --interpret examples/submission_input.nova

# 4. INTERACTIVE / PIPED INPUT (For programs using receive(var); like examples/demo.nova)
echo 42 | ./nova.exe examples/demo.nova
cat output.txt

# 5. INSPECT STAGE 1: FLEX TOKEN STREAM (--tokens)
./nova.exe --tokens examples/submission_input.nova

# 6. INSPECT STAGE 2: BISON SYNTAX CHECK (--parse)
./nova.exe --parse examples/submission_input.nova

# 7. INSPECT STAGE 3: ABSTRACT SYNTAX TREE DUMP (--ast)
./nova.exe --ast examples/submission_input.nova

# 8. INSPECT STAGE 4: LEXICAL SCOPES, SYMBOL TABLE & TYPE CHECKING (--symbols)
./nova.exe --symbols examples/submission_input.nova

# 9. HELP & VERSION
./nova.exe --help
./nova.exe --version
```

---

## 3. File-by-File Architecture Map (What Each File Actually Does)

| File Path | Role in the Compiler | Key Functions / Classes to Know |
|---|---|---|
| [`src/lexer/token.hpp`](../src/lexer/token.hpp) | Defines all token types and source locations | `enum class TokenKind`, `struct SourceLocation` (`filename`, `line`, `column`), `struct Token` |
| [`src/lexer/scanner.l`](../src/lexer/scanner.l) | **Flex Specification (`.l`)** | Matches 34 keywords, operators (`**`, `..`, `->`, `<=`, `>=`, `==`, `!=`), literals, identifiers, and `//` or `/* */` comments; returns `nova::Token` via `MAKE_TOKEN(Kind)`. |
| [`src/lexer/lexer.cpp`](../src/lexer/lexer.cpp) | C++ wrapper around Flex | `Lexer::tokenize()` scans the entire file into `std::vector<Token>` and collects lexical errors. |
| [`src/parser/parser.y`](../src/parser/parser.y) | **Bison Grammar (`.y`)** | Defines `%union`, operator precedence (`%left`, `%right`, `%precedence`), `%destructor` cleanup, and CFG rules that construct AST nodes (`new BinaryExprNode(...)`, etc.). |
| [`src/parser/parser.cpp`](../src/parser/parser.cpp) | Bridge between Flex and Bison | `nova_parser_lex()` feeds tokens from `Parser::consume_token_ref()` to Bison; `nova_parser_error()` formats syntax errors with exact `line:col`. |
| [`src/ast/ast_node.hpp`](../src/ast/ast_node.hpp) | **Abstract Syntax Tree (24 Nodes)** | `TypeSpec`, `ASTNode`, `ExprNode`, `StmtNode`, `ProgramNode`, `RoutineDeclNode`, `ConstDeclNode`, `FluxDeclNode`, `DetectNode`, `OrbitNode`, `CruiseNode`, `RouteNode`, etc. |
| [`src/ast/ast_visitor.hpp`](../src/ast/ast_visitor.hpp) | **Visitor Pattern Interface** | Declares `virtual void visit(<Node>&) = 0;` for all 24 AST nodes so passes are decoupled from AST data structures. |
| [`src/ast/ast_printer.cpp`](../src/ast/ast_printer.cpp) | `--ast` Tree Dumper | Implements `ASTVisitor` to print an indented hierarchical view of the parsed AST. |
| [`src/diagnostics/diagnostic.cpp`](../src/diagnostics/diagnostic.cpp) | Error Reporting Engine | `DiagnosticEngine::report_error()` formats compiler and runtime errors consistently as `file:line:col: error: message`. |
| [`src/semantic/symbol_table.cpp`](../src/semantic/symbol_table.cpp) | Lexical Scope Tree | `Symbol`, `Scope`, and `SymbolTable` manage nested `do ... end` scopes, local frame offsets, variable shadowing, and `--symbols` output. |
| [`src/semantic/symbol_resolver.cpp`](../src/semantic/symbol_resolver.cpp) | Scope & Mutability Pass | Checks undeclared variables/routines, same-scope duplicate declarations, and forbids mutating `const` variables, `const` arrays, or `orbit` loop indices. |
| [`src/semantic/type_checker.cpp`](../src/semantic/type_checker.cpp) | Static Type Checker Pass | Enforces strict typing (no implicit `i32` $\leftrightarrow$ `f32` coercion), compile-time constant array bounds (`arr[5]`, `arr[-1]`), `bool` conditions, `halt`/`skip` loop context, and all-paths `ret` exhaustiveness. |
| [`src/codegen/interpreter.cpp`](../src/codegen/interpreter.cpp) | AST Execution & Output Engine | Evaluates the AST at runtime (`RuntimeValue`), manages call frames for recursion, checks `require`/`verify` contracts, prints `transmit(...)`, reads `receive(...)`, and captures output for `output.txt`. |
| [`src/main.cpp`](../src/main.cpp) | Compiler CLI Driver | Parses CLI flags (`--tokens`, `--parse`, `--ast`, `--symbols`, `--interpret`, `-o`) and orchestrates the pipeline stages. |

---

## 4. How to Justify the Extra Engineering Work to Your Teachers

Teachers may ask: *"Many students just put C code / `printf` directly inside their Bison `.y` actions. Why did you build an AST, Visitor Pattern, Symbol Table, Type Checker, and Interpreter across multiple C++ files?"*

Here are your **4 strongest technical justifications**:

### Justification 1: You Cannot Execute Loops, Branches, or Recursive Functions Inside Bison Actions!
- **Why**: Bison parses the source file **bottom-up in a single pass** as tokens arrive.
  - If you execute statements directly inside Bison grammar reductions (`stmt : transmit_stmt { printf(...); }`), then in an `if/else` (`detect` / `fallback`), **both** the `detect` block and the `fallback` block get reduced before the `detect_stmt` rule finishes—meaning both branches would execute!
  - Even worse, in a `cruise` or `orbit` loop, Bison only parses the loop body **once**. It cannot re-parse tokens 10 times to run a loop, and it cannot jump back to a `routine` body when a recursive function (`factorial(n - 1)`) is called!
- **What our AST does**: Bison's semantic actions build an in-memory **Abstract Syntax Tree (`ProgramNode`)**. Once the tree is built, our `Interpreter` can walk loop bodies (`OrbitNode`, `CruiseNode`) as many iterations as needed, execute only the taken branch of a `DetectNode` or `RouteNode`, and create fresh stack frames for recursive `RoutineDeclNode` calls.

### Justification 2: Catching Semantic Bugs Before Execution (`SymbolResolver` & `TypeChecker`)
- **Why**: A Context-Free Grammar (Bison) can only check **syntax** (whether tokens are in a legal order). A CFG alone cannot check context-sensitive rules like:
  - Whether `x` was declared as immutable `const` or mutable `flux` three blocks ago.
  - Whether `10 + "hello"` or `10 + 2.5` violates static type safety.
  - Whether `arr[5]` is out of bounds on an array declared as `i32[3]`.
  - Whether a non-void function forgets to `ret` a value in its `fallback` branch.
- **What our semantic passes do**: Before running a single line of user code, `SymbolResolver` and `TypeChecker` catch all scope, mutability, array bounds, and type errors with exact `file:line:column` diagnostics.

### Justification 3: Clean Separation of Concerns via the Visitor Pattern (`ASTVisitor`)
- **Why**: Instead of stuffing printing, type-checking, and execution into giant classes inside `parser.y`, `ASTVisitor` ([`src/ast/ast_visitor.hpp`](../src/ast/ast_visitor.hpp)) lets us attach independent compiler passes to the exact same AST:
  - `ASTPrinter` (`--ast`) prints the syntax tree.
  - `SymbolResolver` (`--symbols`) builds the lexical scope tree and checks `const`/`flux`.
  - `TypeChecker` (`--symbols`) validates types, array bounds, and return paths.
  - `Interpreter` (`--interpret` / default) executes the program and writes `output.txt`.

### Justification 4: Design-by-Contract (`require` & `verify`) and Automated 5-State Testing
- **Why**: Inspired by safety-critical aerospace/telemetry software (Eiffel, SPARK Ada), NOVA includes routine preconditions (`require`) and inline invariant assertions (`verify`).
- **Automated Verification**: [`scripts/test_runner.py`](../scripts/test_runner.py) runs **56 automated tests** classifying results into `PASS`, `EXPECTED ERROR`, `UNEXPECTED ERROR`, `FAIL`, and `CRASH` so we know every valid feature works and every invalid program is rejected cleanly without crashing.

---

## 5. Likely Viva Questions & Exact Answers

### Q1: "How did you modify your previous Flex file (`.l`) to work with Bison (`.y`)?"
- **Answer**:
  - In our earlier standalone Flex tool ([`tools/keyword_counter/counter.l`](../tools/keyword_counter/counter.l)), Flex rules incremented counters and printed messages directly inside `main()`.
  - In [`src/lexer/scanner.l`](../src/lexer/scanner.l), we replaced `printf` actions with `MAKE_TOKEN(TokenKind::...)`, which constructs a `nova::Token` object containing the token's enum kind (`KW_IGNITE`, `KW_ROUTINE`, `IDENT`, `INT_LIT`, `OP_POW`, etc.), the matched string (`yytext`), and its 1-indexed `(line, column)` coordinates.
  - In [`src/parser/parser.cpp`](../src/parser/parser.cpp), the bridge function `nova_parser_lex()` hands each token's pointer (`yylval_param->token = &tok`) and source coordinates (`yylloc_param`) directly to Bison's LALR(1) parser.

### Q2: "How did you handle operator precedence and avoid shift/reduce conflicts in Bison?"
- **Answer**:
  - In [`src/parser/parser.y`](../src/parser/parser.y) (lines 188–197), we declared explicit operator precedence and associativity from lowest to highest:
    1. `%left KW_OR` (lowest)
    2. `%left KW_AND`
    3. `%left OP_EQ OP_NEQ`
    4. `%left OP_LT OP_LE OP_GT OP_GE`
    5. `%left OP_ADD OP_SUB`
    6. `%left OP_MUL OP_DIV OP_MOD`
    7. `%precedence UMINUS KW_NOT` (unary `-` and `not`, using `%prec UMINUS` on `OP_SUB expr`)
    8. `%right OP_POW` (`**` is right-associative, and binds tighter than unary `-`, so `-a ** b` parses as `-(a ** b)` and `2 ** 3 ** 2` parses as `2 ** (3 ** 2)` = `512`).
  - Array indexing `arr[i]`, routine calls `fn(x)`, and parentheses `(expr)` sit at the highest level inside `postfix_expr` and `primary_expr`.
  - Because of this hierarchy, Bison compiles `parser.y` with **0 shift/reduce conflicts and 0 reduce/reduce conflicts**.

### Q3: "Why did you use `detect / redetect / fallback` instead of `if / else`? Didn't that avoid the dangling-else problem?"
- **Answer**:
  - Yes! In C-like languages, `if (c1) if (c2) s1; else s2;` causes the classic **dangling-else shift/reduce conflict** because the parser doesn't know which `if` the `else` belongs to.
  - In NOVA, every block is explicitly terminated by `end`:
    ```nova
    detect cond1 do
        ...
    redetect cond2 do
        ...
    fallback do
        ...
    end
    ```
  - Because `KW_END` explicitly closes every `detect_stmt`, `cruise_stmt`, `orbit_stmt`, `route_stmt`, `routine_decl`, and `ignite_block`, the grammar is **100% unambiguous** with zero shift/reduce conflicts!

### Q4: "What is the difference between `const` and `flux`, and where is it enforced?"
- **Answer**:
  - `const` declares an **immutable** binding (mandatory initializer), whereas `flux` declares a **mutable** binding (optional initializer; defaults to `0`, `0.0`, `false`, `'\0'`, `""`, or zero-filled array `T[N]`).
  - It is enforced in [`src/semantic/symbol_resolver.cpp`](../src/semantic/symbol_resolver.cpp) inside `SymbolResolver::check_mutable_storage()`.
  - If a program tries to reassign a `const` variable (`x = 5;`), mutate an element of a `const` array (`const_arr[0] = 5;`), modify an `orbit` loop counter (`cycle = cycle + 1;`), or pass a `const` variable to `receive(x);`, the compiler rejects it with:
    `error: cannot assign to immutable constant 'x'`.

### Q5: "How are arrays (`T[N]`) checked at compile time vs. runtime?"
- **Answer**:
  - **At Compile Time ([`src/semantic/type_checker.cpp`](../src/semantic/type_checker.cpp))**:
    1. Array capacity `N` must be `> 0`.
    2. Array literals `[e0, e1, ...]` must be homogeneous (all elements must have the same type `T`) and match the declared length `N`.
    3. The index expression in `arr[idx]` must have type `i32`.
    4. If the index is a compile-time constant integer expression (e.g., `arr[5]`, `arr[-1]`, `arr[1 + 3]`), `TypeChecker::try_eval_const_i32()` evaluates it at compile time and rejects out-of-bounds indices (`0 <= idx < N`) before the program ever runs!
  - **At Runtime ([`src/codegen/interpreter.cpp`](../src/codegen/interpreter.cpp))**:
    - For dynamic variable indices (`arr[i]`), `Interpreter` checks `0 <= i < arr.elements.size()` at runtime and reports a source-located runtime error if out of bounds.

### Q6: "How does your compiler process an input file and generate the output file?"
- **Answer**:
  - In [`src/main.cpp`](../src/main.cpp) (lines 280–318), running `./nova.exe examples/submission_input.nova`:
    1. Calls `Parser::parse()` (which runs `Lexer::tokenize()` and Bison's `nova_parser_parse()`).
    2. Runs `SymbolResolver::resolve()` and `TypeChecker::check()` on the `ProgramNode` AST. If any lexical, syntax, scope, or type error exists, it prints the diagnostics with `line:col` to `stderr` and exits with code `1` (no bad output file is produced).
    3. Runs `Interpreter::execute()`, which executes all statements in `ignite do ... end` and captures every `transmit(...)` line into an internal buffer (`m_captured_output`) while also streaming to `stdout`.
    4. Writes `interpreter.captured_output()` to `output.txt` (or a custom filename if `-o <file>` was passed).

### Q7: "What if the teacher modifies the input file during the demo to test an error?"
- **Answer**:
  - You can demonstrate both valid execution and error detection immediately:
    - Valid execution: `./nova.exe examples/submission_input.nova` $\rightarrow$ runs and writes `output.txt`.
    - Syntax error test: `./nova.exe --parse tests/parser/missing_semicolon.nova` $\rightarrow$ shows exact line/column and expected token.
    - `const` mutation error test: `./nova.exe --symbols tests/semantic/immutable_assign.nova` $\rightarrow$ shows `cannot assign to immutable constant`.
    - Type / bounds error test: `./nova.exe --symbols tests/semantic/array_type_and_bounds.nova` $\rightarrow$ shows compile-time out-of-bounds error on `arr[3]` and `arr[-1]`.
    - Runtime contract failure test: `./nova.exe --interpret tests/contracts/require_precondition_fail.nova` $\rightarrow$ shows runtime precondition failure on `safe_div(100, 0)`.

---

## 6. Recommended Live Demo Script (2 Minutes)

If asked to demonstrate your project live on your laptop, run these 6 commands in order:

```powershell
# 0. Ensure MSYS2 tools are on PATH
$env:PATH = "C:\msys64\usr\bin;C:\msys64\ucrt64\bin;" + $env:PATH

# 1. Show a clean build from scratch (Flex + Bison + C++17)
make clean && make

# 2. Show the token stream produced by Flex (.l)
./nova.exe --tokens examples/submission_input.nova

# 3. Show the Abstract Syntax Tree built by Bison (.y)
./nova.exe --ast examples/submission_input.nova

# 4. Show the lexical Symbol Table & Static Type Verification
./nova.exe --symbols examples/submission_input.nova

# 5. Execute the submission input file and show the generated output.txt
./nova.exe examples/submission_input.nova
Get-Content output.txt

# 6. Run the 56-test automated verification suite
make test
```
