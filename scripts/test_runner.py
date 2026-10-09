#!/usr/bin/env python3
"""
NOVA Compiler Test Harness
Standard Test Orchestrator adhering to docs/testing.md and AGENTS.md rules.
Classifies test results into: PASS, EXPECTED ERROR, UNEXPECTED ERROR, FAIL, CRASH.
"""

import os
import sys
import glob
import subprocess
from pathlib import Path

# ANSI colors for formatted reporting
GREEN = "\033[92m"
YELLOW = "\033[93m"
RED = "\033[91m"
MAGENTA = "\033[95m"
CYAN = "\033[96m"
BOLD = "\033[1m"
RESET = "\033[0m"

class TestResult:
    PASS = "PASS"
    EXPECTED_ERROR = "EXPECTED ERROR"
    UNEXPECTED_ERROR = "UNEXPECTED ERROR"
    FAIL = "FAIL"
    CRASH = "CRASH"

def find_compiler():
    candidates = [
        Path("nova.exe"),
        Path("nova"),
        Path("bin/nova.exe"),
        Path("bin/nova")
    ]
    for c in candidates:
        if c.is_file():
            return str(c.resolve())
    return None

def normalize_text(text):
    """Normalize newlines and backslashes for cross-platform comparison."""
    if text is None:
        return ""
    return text.replace("\r\n", "\n").replace("\\", "/")

def run_cli_tests(compiler_path):
    """Executes basic CLI behavior tests (Phase 1)."""
    tests = [
        {
            "name": "CLI: Help Command (--help)",
            "args": ["--help"],
            "expected_exit": 0,
            "expected_out_substr": "Usage:",
            "kind": TestResult.PASS
        },
        {
            "name": "CLI: Help Command (-h)",
            "args": ["-h"],
            "expected_exit": 0,
            "expected_out_substr": "NOVA Compiler",
            "kind": TestResult.PASS
        },
        {
            "name": "CLI: Version Command (--version)",
            "args": ["--version"],
            "expected_exit": 0,
            "expected_out_substr": "NOVA Reference Compiler",
            "kind": TestResult.PASS
        },
        {
            "name": "CLI: No Arguments",
            "args": [],
            "expected_exit": 1,
            "expected_err_substr": "error: no input files",
            "kind": TestResult.EXPECTED_ERROR
        },
        {
            "name": "CLI: Unrecognized Option",
            "args": ["--unknown-flag"],
            "expected_exit": 1,
            "expected_err_substr": "error: unrecognized command line option",
            "kind": TestResult.EXPECTED_ERROR
        },
        {
            "name": "CLI: Non-existent Input File",
            "args": ["tests/non_existent_file.nova"],
            "expected_exit": 1,
            "expected_err_substr": "error: cannot open input file",
            "kind": TestResult.EXPECTED_ERROR
        }
    ]

    results = []

    for t in tests:
        cmd = [compiler_path] + t["args"]
        try:
            proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=5)
            stdout = normalize_text(proc.stdout)
            stderr = normalize_text(proc.stderr)
            code = proc.returncode

            # Check for crash
            if code < 0 or code > 128:
                results.append((t["name"], TestResult.CRASH, f"Exited with abnormal code {code}"))
                continue

            if t["kind"] == TestResult.PASS:
                expected_sub = normalize_text(t.get("expected_out_substr", ""))
                if code == t["expected_exit"] and expected_sub in stdout:
                    results.append((t["name"], TestResult.PASS, "Matched expected output and exit code 0"))
                else:
                    results.append((t["name"], TestResult.FAIL, f"Expected exit {t['expected_exit']} with substring '{expected_sub}', got code {code}"))
            elif t["kind"] == TestResult.EXPECTED_ERROR:
                expected_err = normalize_text(t.get("expected_err_substr", ""))
                if code == t["expected_exit"] and expected_err in stderr:
                    results.append((t["name"], TestResult.EXPECTED_ERROR, f"Cleanly reported diagnostic: {stderr.strip()}"))
                else:
                    results.append((t["name"], TestResult.UNEXPECTED_ERROR, f"Expected error code {t['expected_exit']} with '{expected_err}', got {code}"))
        except Exception as e:
            results.append((t["name"], TestResult.CRASH, str(e)))

    return results

def run_file_tests(compiler_path):
    """Parses and executes file-based tests under tests/ directory."""
    results = []
    test_files = glob.glob("tests/**/*.nova", recursive=True)

    for path in sorted(test_files):
        # Parse directives
        kind = TestResult.PASS
        expect_err = None
        expect_out = None
        exit_code = 0
        cli_flags = []

        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line.startswith("//"):
                    break
                if line.startswith("// KIND:"):
                    kind = line.split(":", 1)[1].strip()
                elif line.startswith("// EXPECT_ERR:"):
                    expect_err = line.split(":", 1)[1].strip()
                elif line.startswith("// EXPECT_OUT:"):
                    expect_out = line.split(":", 1)[1].strip()
                elif line.startswith("// EXIT_CODE:"):
                    exit_code = int(line.split(":", 1)[1].strip())
                elif line.startswith("// FLAGS:"):
                    cli_flags = line.split(":", 1)[1].strip().split()

        cmd = [compiler_path] + cli_flags + [path]
        try:
            proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=5)
            stdout = normalize_text(proc.stdout)
            stderr = normalize_text(proc.stderr)
            code = proc.returncode

            display_name = normalize_text(path)

            if code < 0 or code > 128:
                results.append((display_name, TestResult.CRASH, f"Crash with signal/code {code}"))
                continue

            if kind == "PASS":
                expected_out_norm = normalize_text(expect_out) if expect_out else None
                if code == exit_code and (expected_out_norm is None or expected_out_norm in stdout):
                    results.append((display_name, TestResult.PASS, "Matched expected output"))
                else:
                    results.append((display_name, TestResult.FAIL, f"Exit code {code} (expected {exit_code}) or output mismatch"))
            elif kind == "EXPECTED_ERROR":
                expected_err_norm = normalize_text(expect_err) if expect_err else None
                if code == exit_code and (expected_err_norm is None or expected_err_norm in stderr):
                    results.append((display_name, TestResult.EXPECTED_ERROR, f"Diagnostic: {stderr.strip()}"))
                else:
                    results.append((display_name, TestResult.UNEXPECTED_ERROR, f"Expected '{expected_err_norm}', got exit {code}"))
        except Exception as e:
            results.append((normalize_text(path), TestResult.CRASH, str(e)))

    return results

def main():
    compiler_path = find_compiler()
    if not compiler_path:
        print(f"{RED}Error: NOVA compiler binary not found. Run 'make' first.{RESET}")
        sys.exit(1)

    print(f"{CYAN}{BOLD}=================================================={RESET}")
    print(f"{CYAN}{BOLD}       NOVA COMPILER TEST HARNESS                 {RESET}")
    print(f"{CYAN}{BOLD}=================================================={RESET}")
    print(f"Compiler binary: {compiler_path}\n")

    cli_results = run_cli_tests(compiler_path)
    file_results = run_file_tests(compiler_path)
    all_results = cli_results + file_results

    counts = {
        TestResult.PASS: 0,
        TestResult.EXPECTED_ERROR: 0,
        TestResult.UNEXPECTED_ERROR: 0,
        TestResult.FAIL: 0,
        TestResult.CRASH: 0,
    }

    for name, status, detail in all_results:
        counts[status] += 1
        if status == TestResult.PASS:
            tag = f"{GREEN}[PASS]{RESET}"
        elif status == TestResult.EXPECTED_ERROR:
            tag = f"{YELLOW}[EXPECTED ERROR]{RESET}"
        elif status == TestResult.UNEXPECTED_ERROR:
            tag = f"{RED}[UNEXPECTED ERROR]{RESET}"
        elif status == TestResult.FAIL:
            tag = f"{RED}[FAIL]{RESET}"
        elif status == TestResult.CRASH:
            tag = f"{MAGENTA}[CRASH]{RESET}"

        print(f"  {tag:<26} {name:<45} : {detail}")

    print(f"\n{CYAN}{BOLD}--------------------------------------------------{RESET}")
    print(f"{BOLD}SUMMARY:{RESET}")
    print(f"  {GREEN}PASS            : {counts[TestResult.PASS]}{RESET}")
    print(f"  {YELLOW}EXPECTED ERROR  : {counts[TestResult.EXPECTED_ERROR]}{RESET}")
    print(f"  {RED}UNEXPECTED ERROR: {counts[TestResult.UNEXPECTED_ERROR]}{RESET}")
    print(f"  {RED}FAIL            : {counts[TestResult.FAIL]}{RESET}")
    print(f"  {MAGENTA}CRASH           : {counts[TestResult.CRASH]}{RESET}")
    print(f"  TOTAL TESTS     : {len(all_results)}")
    print(f"{CYAN}{BOLD}--------------------------------------------------{RESET}")

    if counts[TestResult.FAIL] > 0 or counts[TestResult.UNEXPECTED_ERROR] > 0 or counts[TestResult.CRASH] > 0:
        print(f"{RED}{BOLD}TEST SUITE RESULT: FAILED{RESET}")
        sys.exit(1)
    else:
        print(f"{GREEN}{BOLD}TEST SUITE RESULT: PASSED ALL VERIFICATIONS{RESET}")
        sys.exit(0)

if __name__ == "__main__":
    main()
