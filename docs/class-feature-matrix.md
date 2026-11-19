# NOVA Language — Class Feature & Test Traceability Matrix

This document maps every authoritative language feature specified in [`docs/language-spec.md`](language-spec.md) and [`docs/grammar.md`](grammar.md) to its compiler implementation modules and automated verification test suites.

---

## 1. Lexical & Syntactic Foundations

| Feature Category | Language Syntax / Construct | Compiler Modules | Verification Tests |
|---|---|---|---|
| **34 Reserved Keywords** | `ignite`, `routine`, `ret`, `const`, `flux`, `do`, `end`, `i32`, `f32`, `bool`, `char`, `str`, `void`, `detect`, `redetect`, `fallback`, `route`, `case`, `default`, `orbit`, `in`, `step`, `cruise`, `halt`, `skip`, `transmit`, `receive`, `verify`, `require`, `and`, `or`, `not`, `true`, `false` | `src/lexer/token.hpp`<br>`src/lexer/scanner.l`<br>`tools/keyword_counter/counter.l` | `tests/lexer/keywords.nova`<br>`examples/demo.nova` (`34/34` in `keyword_counts.txt`) |
| **Comments & Coordinates** | Single-line `// ...` and multi-line `/* ... */` with 1-indexed `line:col` tracking | `src/lexer/scanner.l`<br>`src/lexer/lexer.cpp` | `tests/lexer/comments.nova`<br>`tests/lexer/line_numbers.nova` |
| **Literals & Identifiers** | `INT_LIT`, `FLOAT_LIT`, `CHAR_LIT`, `STR_LIT`, `true`/`false`, `[a-zA-Z_][a-zA-Z0-9_]*` | `src/lexer/scanner.l` | `tests/lexer/literals.nova`<br>`tests/lexer/identifiers.nova`<br>`tests/lexer/invalid_character.nova`<br>`tests/lexer/invalid_literal.nova` |
| **Program Capsule** | `ignite do ... end` root execution block | `src/parser/parser.y`<br>`src/ast/ast_node.hpp` | `tests/parser/ignite.nova`<br>`tests/parser/missing_ignite.nova`<br>`tests/parser/missing_end.nova` |

---

## 2. Type System & State Bindings

| Feature Category | Language Syntax / Construct | Compiler Modules | Verification Tests |
|---|---|---|---|
| **Primitive Types** | `i32`, `f32`, `bool`, `char`, `str`, `void` (strict static typing, no implicit coercions) | `src/ast/ast_node.hpp`<br>`src/semantic/type_checker.cpp` | `tests/semantic/valid_types.nova`<br>`tests/semantic/type_mismatch_expr.nova` |
| **Fixed-Size Arrays (`T[N]`)** | Declaration `T[N]`, homogeneous literals `[e0, ..., eN-1]`, indexing `arr[i]`, compile-time & runtime bounds verification | `src/parser/parser.y`<br>`src/semantic/type_checker.cpp`<br>`src/codegen/interpreter.cpp` | `tests/lexer/arrays.nova`<br>`tests/parser/routines_and_arrays.nova`<br>`tests/parser/invalid_array_decl.nova`<br>`tests/semantic/array_type_and_bounds.nova`<br>`tests/contracts/runtime_bounds_and_div_zero.nova` |
| **Immutable State (`const`)** | `const id: type = expr;` (mandatory initializer; rejects scalar reassignment, array element mutation, and `receive`) | `src/semantic/symbol_resolver.cpp` | `tests/ast/declarations.nova`<br>`tests/semantic/immutable_assign.nova` |
| **Mutable State (`flux`)** | `flux id: type [= expr];` (optional initializer with deterministic zero-value defaults; supports scalar & element mutation) | `src/semantic/symbol_resolver.cpp`<br>`src/codegen/interpreter.cpp` | `tests/ast/declarations.nova`<br>`tests/integration/data_types_and_arrays_io.nova` |
| **Lexical Scoping & Shadowing** | Nested `do ... end` scopes, inner shadowing allowed, same-scope redeclaration rejected | `src/semantic/symbol_table.cpp`<br>`src/semantic/symbol_resolver.cpp` | `tests/semantic/valid_scoping.nova`<br>`tests/semantic/duplicate_decl.nova`<br>`tests/semantic/undeclared_symbol.nova` |

---

## 3. Expressions & 9-Level Operator Precedence

| Precedence Level | Operators | Associativity | Verification Tests |
|---|---|---|---|
| **1 (Highest)** | Grouping `()`, routine call `f(...)`, array index `a[i]` | Left-to-right | `tests/expressions/complex_equations_and_indexing.nova` |
| **2** | Exponentiation `**` | Right-to-left | `tests/expressions/arithmetic_associativity.nova`<br>`tests/expressions/unary_and_power.nova` |
| **3** | Unary `-`, logical `not` | Right-to-left | `tests/expressions/unary_and_power.nova` |
| **4** | Multiplicative `*`, `/`, `%` (`%` strictly `i32`) | Left-to-right | `tests/expressions/arithmetic_associativity.nova`<br>`tests/semantic/type_mismatch_expr.nova` |
| **5** | Additive `+`, `-` | Left-to-right | `tests/expressions/arithmetic_associativity.nova` |
| **6** | Relational `<`, `<=`, `>`, `>=` | Left-to-right | `tests/expressions/boolean_and_comparison.nova` |
| **7** | Equality `==`, `!=` | Left-to-right | `tests/expressions/boolean_and_comparison.nova` |
| **8** | Logical `and` (short-circuit) | Left-to-right | `tests/expressions/boolean_and_comparison.nova` |
| **9 (Lowest)** | Logical `or` (short-circuit) | Left-to-right | `tests/expressions/boolean_and_comparison.nova` |

---

## 4. Control Flow, Routines, Contracts & Telemetry I/O

| Feature Category | Language Syntax / Construct | Compiler Modules | Verification Tests |
|---|---|---|---|
| **Detection Branching** | `detect <bool> do ... redetect <bool> do ... fallback do ... end` | `src/semantic/type_checker.cpp`<br>`src/codegen/interpreter.cpp` | `tests/control_flow/nested_branches_and_loops.nova`<br>`tests/semantic/condition_and_call_types.nova` |
| **Cruise Loop** | `cruise <bool> do ... end` | `src/semantic/type_checker.cpp`<br>`src/codegen/interpreter.cpp` | `tests/control_flow/nested_branches_and_loops.nova` |
| **Orbit Stepped Loop** | `orbit <id> in <start>..<end> [step <s>] do ... end` (`<id>` scoped as `const i32`; rejects `step 0`) | `src/semantic/symbol_resolver.cpp`<br>`src/semantic/type_checker.cpp`<br>`src/codegen/interpreter.cpp` | `tests/control_flow/orbit_and_route_variants.nova`<br>`tests/control_flow/invalid_route_and_orbit.nova` |
| **Route Dispatch** | `route <i32\|char\|bool> do case <lit>: ... default: ... end` (rejects duplicate cases) | `src/semantic/type_checker.cpp`<br>`src/codegen/interpreter.cpp` | `tests/control_flow/orbit_and_route_variants.nova`<br>`tests/control_flow/invalid_route_and_orbit.nova` |
| **Cycle Control** | `halt;` and `skip;` (enforced inside `cruise`/`orbit` loops only) | `src/semantic/type_checker.cpp`<br>`src/codegen/interpreter.cpp` | `tests/control_flow/nested_branches_and_loops.nova`<br>`tests/control_flow/invalid_halt_skip.nova` |
| **Routines & Recursion** | `routine <id>(<params>) [-> <type>] do [require ...;] ... ret [expr]; end` (direct/mutual recursion, all-paths return exhaustiveness) | `src/semantic/symbol_resolver.cpp`<br>`src/semantic/type_checker.cpp`<br>`src/codegen/interpreter.cpp` | `tests/functions/valid_routines_and_recursion.nova`<br>`tests/functions/exhaustive_branch_returns.nova`<br>`tests/functions/invalid_arity_and_arg_types.nova`<br>`tests/functions/invalid_returns.nova` |
| **Contracts** | `require <bool>;` (routine preconditions) and `verify <bool>;` (runtime invariants) | `src/semantic/type_checker.cpp`<br>`src/codegen/interpreter.cpp` | `tests/contracts/verify_and_require_pass.nova`<br>`tests/contracts/require_precondition_fail.nova`<br>`tests/contracts/verify_invariant_fail.nova` |
| **Telemetry I/O & Output File** | Multi-arg `transmit(e1, e2, ...);` (`i32`, `f32`, `bool`, `char`, `str`, `T[N]`), `receive(flux_var);`, and automatic `output.txt` / `-o <file>` generation | `src/codegen/interpreter.cpp`<br>`src/main.cpp` | `tests/integration/telemetry_end_to_end.nova`<br>`tests/integration/data_types_and_arrays_io.nova`<br>`examples/demo.nova` |
