# NOVA Testing Protocol & Harness Specification

This document defines the testing methodology, directory layout, execution harness, and result classification standards for the **NOVA Compiler** project.

---

## 1. Test Harness Invocation

Testing is integrated into the build automation system. The test suite is invoked via:

```bash
make test
```

The Makefile delegates test orchestration to an automated test runner script (`scripts/test_runner.py` or shell equivalent), which executes test cases, compares actual outputs against expected behaviors, and generates a formatted summary.

---

## 2. Test Result Classification

Every test case executed must be classified into one of five mutually exclusive states:

| Classification | Meaning |
|---|---|
| **PASS** | Valid program compiled/executed successfully, and output matched expected output. |
| **EXPECTED ERROR** | Negative test deliberately containing a lexical, syntax, or semantic error was rejected cleanly with the expected diagnostic message and non-zero exit code. |
| **UNEXPECTED ERROR** | A test failed unexpectedly: either a valid test was rejected, or an invalid test failed with the wrong error type/message. |
| **FAIL** | Output mismatch between expected and actual result. |
| **CRASH** | Compiler aborted with a segmentation fault, uncaught exception, or abnormal signal. |

*Crucial Rule*: A negative test **never** passes simply because the compiler returned a non-zero exit code. It passes only if the diagnostic output matches the anticipated error class (e.g. `undeclared identifier`, `type mismatch`, `syntax error`).

---

## 3. Test Directory Organization

Test cases are organized strictly by compiler pipeline phase:

```text
tests/
├── lexer/             # Tokenization tests
│   ├── keywords.nova
│   ├── operators.nova
│   ├── literals.nova
│   ├── invalid_character.nova
│   └── line_numbers.nova
├── parser/            # Syntactic correctness & recovery
│   ├── ignite.nova
│   ├── missing_end.nova
│   └── unexpected_token.nova
├── ast/               # Tree representation checks
│   ├── expressions.nova
│   └── declarations.nova
├── semantic/          # Scoping, immutability, and typing
│   ├── valid_scopes.nova
│   ├── immutable_assign.nova
│   ├── undeclared_var.nova
│   └── type_mismatch.nova
├── control_flow/      # if, while, orbit, route constructs
│   ├── orbit_cycles.nova
│   └── route_dispatch.nova
├── functions/         # Procedures, call conventions, recursion, arity
│   ├── recursion.nova
│   └── arity_mismatch.nova
├── contracts/         # Runtime verify & require checks
│   ├── verify_pass.nova
│   └── verify_fail.nova
├── ir/                # Intermediate code verification
│   └── tac_emission.nova
├── optimization/      # Optimization correctness & safety
│   ├── const_prop.nova
│   └── contract_dce.nova
└── integration/       # End-to-end benchmark programs
    ├── fibonacci.nova
    └── telemetry_calc.nova
```

---

## 4. Test Case Format & Conventions

Test files are standard `.nova` source files with embedded directive headers:

```nova
// TEST: tests/semantic/immutable_assign.nova
// KIND: EXPECTED_ERROR
// EXPECT_ERR: cannot assign to immutable constant 'x'
// EXIT_CODE: 1

ignite do
    const x: i32 = 10;
    x = 20; // Error
end
```

For positive tests:
```nova
// TEST: tests/integration/telemetry_calc.nova
// KIND: PASS
// EXPECT_OUT: 120
// EXIT_CODE: 0

proc factorial(n: i32) -> i32 do
    if n <= 1 do
        ret 1;
    end
    ret n * factorial(n - 1);
end

ignite do
    transmit(factorial(5));
end
```

---

## 5. Phase Verification Protocol

Before completing any implementation phase, the developer or AI agent must produce the following verification report in the chat record and documentation log:

```text
==================================================
PHASE VERIFICATION REPORT
==================================================
PHASE: <Phase Number and Title>
IMPLEMENTED: <Concise summary of deliverables>
FILES CREATED/MODIFIED:
  - <path/to/file1>
  - <path/to/file2>
TEST FILES CREATED:
  - <path/to/test1.nova>
COMMANDS EXECUTED:
  - <make clean>
  - <make>
  - <make test>
BUILD STATUS: PASS / FAIL
NEW TEST RESULTS: <N passed, M expected errors>
REGRESSION RESULTS: <All prior phase tests passed>
KNOWN LIMITATIONS: <Any deferred items>
MANUAL TESTING INSTRUCTIONS:
  <Exact CLI invocations to reproduce verification>
==================================================
```
