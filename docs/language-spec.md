# NOVA Language Specification

**Version:** 1.0 (Draft)  
**Status:** Authoritative Document  
**Target:** NOVA Reference Compiler (Flex / Bison / C++17)

---

## 1. Overview and Design Philosophy

NOVA is a statically typed, compiled programming language designed to combine modern readability with conventional compiler design principles. It eschews C-style curly braces in favor of explicit `do ... end` block structures, enforces explicit mutability at the variable declaration level (`val` vs `var`), and incorporates contract-oriented verification constructs (`assert`, `require`).

---

## 2. Lexical Structure

### 2.1 Character Set & Whitespace
Source code is encoded in UTF-8 / ASCII. Whitespace consists of spaces (`0x20`), tabs (`\t`), carriage returns (`\r`), and newlines (`\n`). Whitespace serves solely to delimit tokens and is otherwise ignored outside character and string literals.

### 2.2 Comments
- **Single-line comment**: Begins with `//` and continues to the end of the line.
- **Multi-line comment**: Begins with `/*` and ends with `*/`. Multi-line comments do not nest.

### 2.3 Identifiers
An identifier begins with an ASCII letter (`a`-`z`, `A`-`Z`) or an underscore (`_`), followed by zero or more letters, underscores, or digits (`0`-`9`).
```regex
[a-zA-Z_][a-zA-Z0-9_]*
```

### 2.4 Keywords
The following tokens are reserved keywords and cannot be used as identifiers:

| Category | Keywords |
|---|---|
| Program Structure | `entry`, `do`, `end`, `fn`, `ret` |
| Variable Binding | `val`, `var` |
| Primitive Types | `i32`, `f32`, `bool`, `char`, `str`, `void` |
| Control Flow | `if`, `elif`, `else`, `select`, `case`, `default` |
| Loops | `loop`, `in`, `step`, `while`, `break`, `next` |
| I/O Operations | `emit`, `read` |
| Contracts | `assert`, `require` |
| Logical Operators | `and`, `or`, `not` |
| Boolean Literals | `true`, `false` |

### 2.5 Literals
- **Integer (`i32`)**: Sequence of decimal digits (`[0-9]+`), e.g. `0`, `42`, `1000`.
- **Floating-point (`f32`)**: Sequence of digits with a decimal point (`[0-9]+\.[0-9]+`), e.g. `3.14`, `0.0`, `100.5`.
- **Boolean (`bool`)**: `true` or `false`.
- **Character (`char`)**: Single character enclosed in single quotes, e.g. `'a'`, `'\n'`, `'\\'`.
- **String (`str`)**: Sequence of characters enclosed in double quotes, e.g. `"Hello, NOVA!\n"`. Escape sequences supported: `\n`, `\t`, `\r`, `\\`, `\"`, `\'`, `\0`.

### 2.6 Operators and Delimiters
- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `**` (exponentiation)
- **Relational**: `<`, `<=`, `>`, `>=`, `==`, `!=`
- **Assignment**: `=`
- **Range**: `..`
- **Punctuation**: `:`, `;`, `,`, `.`, `->`, `(`, `)`, `[`, `]`

---

## 3. Type System

NOVA is strictly and statically typed. No implicit type promotions or coercions are permitted.

### 3.1 Built-in Primitive Types
1. **`i32`**: 32-bit signed two's complement integer (range: $-2^{31}$ to $2^{31} - 1$).
2. **`f32`**: 32-bit IEEE 754 single-precision floating-point number.
3. **`bool`**: Boolean type taking either `true` or `false`.
4. **`char`**: Single ASCII/byte character.
5. **`str`**: Immutable string literal or string sequence.
6. **`void`**: Unit type representing the absence of a value (only valid as function return type).

### 3.2 Type Equivalence & Compatibility
- Binary arithmetic operators (`+`, `-`, `*`, `/`, `%`, `**`) require both operands to have identical numeric types (`i32` with `i32`, or `f32` with `f32`). `%` is only defined on `i32`.
- Relational operators (`<`, `<=`, `>`, `>=`) require operands of matching numeric types (`i32` or `f32`) and yield a `bool`.
- Equality operators (`==`, `!=`) operate on matching types (`i32`, `f32`, `bool`, `char`, `str`) and yield a `bool`.
- Logical operators (`and`, `or`, `not`) strictly require `bool` operands.

---

## 4. Variables and Scoping

### 4.1 Declarations
- **Immutable Variable (`val`)**:
  ```nova
  val <identifier>: <type> = <expression>;
  ```
  The initializer expression is mandatory. Reassignment to a `val` is a compile-time semantic error.

- **Mutable Variable (`var`)**:
  ```nova
  var <identifier>: <type> [= <expression>];
  ```
  The initializer is optional. If omitted, the variable is default-initialized:
  - `i32`: `0`
  - `f32`: `0.0`
  - `bool`: `false`
  - `char`: `'\0'`
  - `str`: `""`

### 4.2 Scoping and Shadowing
- Every block delimited by `do ... end` introduces a new lexical scope.
- Variables declared within an inner scope shadow variables of the same name in enclosing outer scopes.
- Declaring two variables with identical identifiers in the same scope level is a compile-time error.
- Variables are visible only after their point of declaration.

---

## 5. Expressions and Precedence

Operators are evaluated in order of precedence. Parentheses `(...)` override precedence.

| Precedence | Operator | Associativity | Description |
|---|---|---|---|
| 1 (Highest) | `()`, function call `f(...)` | Left-to-right | Grouping, Call |
| 2 | `**` | Right-to-left | Exponentiation |
| 3 | `-` (unary), `not` | Right-to-left | Unary negation, Logical NOT |
| 4 | `*`, `/`, `%` | Left-to-right | Multiplicative |
| 5 | `+`, `-` | Left-to-right | Additive |
| 6 | `<`, `<=`, `>`, `>=` | Left-to-right | Relational |
| 7 | `==`, `!=` | Left-to-right | Equality |
| 8 | `and` | Left-to-right | Logical AND (short-circuit) |
| 9 (Lowest) | `or` | Left-to-right | Logical OR (short-circuit) |

---

## 6. Statements and Control Flow

### 6.1 Program Structure
A valid NOVA program consists of optional top-level function declarations followed by exactly one `entry` block:

```nova
// Optional function declarations
fn add(a: i32, b: i32) -> i32 do
    ret a + b;
end

// Mandatory program entry point
entry do
    val x: i32 = 10;
    emit(x);
end
```

### 6.2 Conditionals
```nova
if condition do
    // statements
elif condition2 do
    // statements
else do
    // statements
end
```
- `elif` and `else` blocks are optional.
- Multiple `elif` blocks are permitted.
- The condition must evaluate to type `bool`.

### 6.3 While Loops
```nova
while condition do
    // statements
end
```
- The condition must evaluate to type `bool`.
- Loop body executes zero or more times while `condition` remains `true`.

### 6.4 Counted Loops
```nova
loop i in start..end [step s] do
    // statements
end
```
- `start` and `end` must evaluate to `i32`.
- `step s` is optional; if omitted, the step defaults to `1`.
- The loop index variable `i` is implicitly declared as an immutable `val i: i32` local to the loop body for each iteration.

### 6.5 Select-Case Statements
```nova
select expression do
    case val1:
        // statements
    case val2:
        // statements
    default:
        // statements
end
```
- `expression` must evaluate to `i32`, `char`, or `bool`.
- Each case label must be a compile-time literal constant matching the selector expression's type.
- Case labels must be unique within the `select` statement.
- Execution breaks automatically after executing a matched case (no implicit fallthrough).
- `default:` is optional.

### 6.6 Jump Statements
- `break;`: Terminates the innermost enclosing `while` or `loop`.
- `next;`: Immediately skips to the next iteration of the innermost enclosing `while` or `loop`.
- Using `break;` or `next;` outside a loop is a compile-time error.

---

## 7. Functions

### 7.1 Declaration Syntax
```nova
fn <name>(<param1>: <type1>, <param2>: <type2>) -> <return_type> do
    // body statements
    ret <expression>;
end
```
For functions returning `void`:
```nova
fn <name>(<param1>: <type1>) do
    // body statements
    ret; // optional
end
```

### 7.2 Semantics
- Functions must be declared at the top-level before `entry`.
- Forward declarations are not required if mutually recursive functions are supported via multi-pass analysis.
- Parameters are passed by value.
- Non-void functions must guarantee that every control path returns an expression of the declared return type.
- Void functions may return with `ret;` or exit at the end of the block.

---

## 8. Input and Output

### 8.1 Output (`emit`)
```nova
emit(<expression>);
```
Evaluates `<expression>` and prints its string representation to standard output followed by a newline. Supported for `i32`, `f32`, `bool`, `char`, and `str`.

### 8.2 Input (`read`)
```nova
read(<variable_identifier>);
```
Reads a whitespace-delimited token or line from standard input, parses it according to the variable's type, and stores the value in `<variable_identifier>`. The target variable must be mutable (`var`).

---

## 9. Contract and Verification System

NOVA integrates formal assertions into the language syntax for static verification and runtime safety.

### 9.1 Runtime Assertions (`assert`)
```nova
assert <condition>;
```
- `<condition>` must evaluate to `bool`.
- At runtime, if `<condition>` evaluates to `false`, program execution is immediately halted with a descriptive assertion failure and line location.

### 9.2 Function Preconditions (`require`)
```nova
fn divide(a: f32, b: f32) -> f32 do
    require b != 0.0;
    ret a / b;
end
```
- `require` statements must appear at the top of a function's body before any other statements.
- Checked upon function invocation. If violated, execution terminates with a precondition error.
- Used in later phases for contract propagation and optimization in the compiler IR.

---

## 10. Diagnostics and Error Handling

1. **Lexical Errors**: Unterminated strings, invalid characters, or malformed numbers produce errors specifying file, line, and column numbers.
2. **Syntax Errors**: Unmatched delimiters, misplaced keywords, or malformed statements produce actionable Bison syntax error reports.
3. **Semantic Errors**: Type mismatches, undeclared variables, duplicate bindings in the same scope, reassignments to `val`, or return type mismatches are reported with source coordinates before any code generation occurs.
