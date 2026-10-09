# NOVA Language Specification

**Version:** 1.0  
**Theme:** Cosmic Telemetry & Systems Architecture  
**Status:** Authoritative Specification  
**Target:** NOVA Reference Compiler (Flex / Bison / C++17)

---

## 1. Overview and Design Philosophy

NOVA is a statically typed, compiled programming language designed with a **cosmic telemetry & systems computing** aesthetic. It blends futuristic, high-tech syntax with solid, unambiguous compiler theory:
- **`ignite do ... end`**: Explicit root execution capsule.
- **`proc`**: High-efficiency procedural routines with explicit contracts.
- **`const` / `flux`**: Explicit state classification (immutable vs. mutable/dynamic).
- **`orbit`**: Deterministic stepped loop cycles.
- **`transmit` / `receive`**: Telemetry-styled streams for standard I/O.
- **`verify` / `require`**: First-class invariant assertions and function preconditions.

---

## 2. Lexical Structure

### 2.1 Character Set & Whitespace
Source code is encoded in UTF-8 / ASCII. Whitespace consists of spaces (`0x20`), tabs (`\t`), carriage returns (`\r`), and newlines (`\n`). Whitespace serves solely to delimit tokens and is otherwise ignored outside string and character literals.

### 2.2 Comments
- **Single-line comment**: Begins with `//` and extends to the end of the current line.
- **Multi-line comment**: Begins with `/*` and ends with `*/`. Multi-line comments do not nest.

### 2.3 Identifiers
An identifier begins with an ASCII letter (`a`-`z`, `A`-`Z`) or an underscore (`_`), followed by zero or more letters, underscores, or digits (`0`-`9`).
```regex
[a-zA-Z_][a-zA-Z0-9_]*
```

### 2.4 Keywords
The following tokens are reserved keywords:

| Category | Keywords |
|---|---|
| Execution Capsule | `ignite`, `do`, `end`, `proc`, `ret` |
| State Binding | `const` (immutable), `flux` (mutable) |
| Primitive Types | `i32`, `f32`, `bool`, `char`, `str`, `void` |
| Control Flow | `if`, `elif`, `else`, `route`, `case`, `default` |
| Execution Cycles | `orbit`, `in`, `step`, `cruise`, `halt`, `skip` |
| Telemetry I/O | `transmit`, `receive` |
| Contracts & Safety | `verify`, `require` |
| Logical Operators | `and`, `or`, `not` |
| Boolean Literals | `true`, `false` |

### 2.5 Literals
- **Integer (`i32`)**: Sequence of decimal digits (`[0-9]+`), e.g. `0`, `42`, `1000`.
- **Floating-point (`f32`)**: Decimal representation (`[0-9]+\.[0-9]+`), e.g. `3.1415`, `0.0`, `128.5`.
- **Boolean (`bool`)**: `true` or `false`.
- **Character (`char`)**: Single ASCII byte enclosed in single quotes, e.g. `'a'`, `'\n'`, `'\\'`. Escapes: `\n`, `\t`, `\r`, `\\`, `\'`, `\0`.
- **String (`str`)**: Sequence of characters enclosed in double quotes, e.g. `"NOVA Telemetry Online\n"`.

### 2.6 Operators and Delimiters
- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `**` (exponentiation)
- **Relational**: `<`, `<=`, `>`, `>=`, `==`, `!=`
- **Assignment**: `=`
- **Range**: `..`
- **Punctuation**: `:`, `;`, `,`, `.`, `->`, `(`, `)`, `[`, `]`

---

## 3. Type System

NOVA is strictly and statically typed. No implicit type promotions or coercions are permitted.

### 3.1 Built-in Types
1. **`i32`**: 32-bit signed two's complement integer.
2. **`f32`**: 32-bit IEEE 754 single-precision float.
3. **`bool`**: Boolean logical value (`true`, `false`).
4. **`char`**: 8-bit character byte.
5. **`str`**: Immutable UTF-8 string literal or sequence.
6. **`void`**: Unit type representing the absence of return data.

### 3.2 Compatibility Rules
- Arithmetic binary operations (`+`, `-`, `*`, `/`, `%`, `**`) require identical numeric types on both sides. Modulo `%` requires `i32`.
- Relational operators (`<`, `<=`, `>`, `>=`) require identical numeric types and yield `bool`.
- Equality operators (`==`, `!=`) operate on matching types and yield `bool`.
- Logical operators (`and`, `or`, `not`) strictly accept and yield `bool`.

---

## 4. State Binding & Scoping

### 4.1 State Declaration
- **Constant Binding (`const`)**:
  ```nova
  const <identifier>: <type> = <expression>;
  ```
  Initial value is mandatory. Reassignment to a `const` is a compile-time semantic error.

- **Flux Binding (`flux`)**:
  ```nova
  flux <identifier>: <type> [= <expression>];
  ```
  Represents dynamic, mutable state. Initializer is optional; if omitted, defaults to zero-value (`0`, `0.0`, `false`, `'\0'`, `""`).

### 4.2 Scoping & Shadowing
- Every `do ... end` block creates an isolated lexical scope.
- Inner declarations may shadow variables from outer enclosing scopes.
- Re-declaring an identifier within the exact same scope is a compile-time semantic error.

---

## 5. Expressions & Precedence

Precedence levels from highest to lowest:

| Precedence | Operator | Associativity | Description |
|---|---|---|---|
| 1 (Highest) | `()`, function call `p(...)` | Left-to-right | Grouping, Procedure Call |
| 2 | `**` | Right-to-left | Exponentiation |
| 3 | `-` (unary), `not` | Right-to-left | Unary negation, Logical NOT |
| 4 | `*`, `/`, `%` | Left-to-right | Multiplicative |
| 5 | `+`, `-` | Left-to-right | Additive |
| 6 | `<`, `<=`, `>`, `>=` | Left-to-right | Relational |
| 7 | `==`, `!=` | Left-to-right | Equality |
| 8 | `and` | Left-to-right | Logical AND (short-circuit) |
| 9 (Lowest) | `or` | Left-to-right | Logical OR (short-circuit) |

---

## 6. Statements & Control Flow

### 6.1 Program Structure
A NOVA program consists of zero or more top-level procedure declarations followed by the mandatory execution capsule `ignite`:

```nova
proc calculate_thrust(mass: f32, accel: f32) -> f32 do
    require mass > 0.0;
    ret mass * accel;
end

ignite do
    const payload: f32 = 1200.0;
    flux thrust: f32 = calculate_thrust(payload, 9.8);
    transmit(thrust);
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

### 6.3 Cruise Cycles (Conditional Loop)
```nova
cruise condition do
    // statements
end
```

### 6.4 Orbit Cycles (Counted Loops)
```nova
orbit cycle_index in start..end [step s] do
    // statements
end
```
- `start` and `end` must evaluate to `i32`.
- `step s` is optional (defaults to `1`).
- `cycle_index` is implicitly scoped within the loop block as an immutable `const i32`.

### 6.5 Route Dispatch (Switch)
```nova
route signal_code do
    case 101:
        transmit("Telemetry OK");
    case 404:
        transmit("Signal Lost");
    default:
        transmit("Unknown Signal");
end
```
- The dispatch expression must be `i32`, `char`, or `bool`.
- Cases do not fall through.

### 6.6 Cycle Interruption
- `halt;`: Immediately breaks and exits the innermost loop.
- `skip;`: Immediately advances to the next iteration of the innermost loop.

---

## 7. Procedures (`proc`)

### 7.1 Syntax
```nova
proc <name>(<param1>: <type1>, <param2>: <type2>) -> <return_type> do
    opt_require_contracts;
    // procedure body
    ret <expression>;
end
```
For void procedures:
```nova
proc <name>(<param1>: <type1>) do
    // procedure body
    ret; // optional
end
```

---

## 8. Telemetry I/O

### 8.1 Transmission (`transmit`)
```nova
transmit(<expression>);
```
Outputs `<expression>` to standard output followed by a newline.

### 8.2 Reception (`receive`)
```nova
receive(<flux_variable>);
```
Scans standard input and stores the parsed value into `<flux_variable>`. The target must be a mutable `flux` binding.

---

## 9. Contracts and Invariant Verification

### 9.1 Invariant Verification (`verify`)
```nova
verify <condition>;
```
Evaluates `<condition>` (`bool`). If `false`, runtime halts immediately with an invariant violation report and source coordinates.

### 9.2 Precondition Requirements (`require`)
```nova
proc orbital_velocity(r: f32) -> f32 do
    require r > 0.0;
    ret 398600.4 / r;
end
```
Checked immediately upon procedure invocation.
