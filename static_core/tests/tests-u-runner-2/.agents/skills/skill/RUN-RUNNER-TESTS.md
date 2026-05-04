---
name: run-runner-tests
description: Running the unit and functional tests of the runner itself (runner-test.sh)
---

# Skill: Run Runner Self-Tests

## Overview

This skill covers how to run the unit and functional tests of URunner-2 itself.
These tests live in `runner/test/` and verify the runner's own logic — config loading,
macro expansion, validators, generators, metadata parsing, and more.
Use this skill whenever you change runner code and need to verify nothing is broken.

---

## Step 1 — Run All Runner Tests

**Option A — via `runner-test.sh` (recommended, activates the virtual environment automatically):**

```bash
./runner-test.sh
```

The script:
1. Activates the Python virtual environment via `scripts/python/venv-utils.sh`.
2. Runs `python3 -B -m unittest discover -s runner/test -p "*_test*.py"`.
3. Exits with the test suite's exit code (0 = all passed, non-zero = failures).

**Option B — via `tox` (when the runner is installed as a package):**

```bash
source ~/.venv-panda/bin/activate
tox run -e unittests
deactivate
```

`tox` also generates a coverage report after the test run.

**Option C — directly with `unittest` (manual venv activation):**

```bash
source ~/.venv-panda/bin/activate
python3 -B -m unittest discover -s runner/test -p "*_test*.py"
deactivate
```

---

## Step 2 — Run a Specific Test Module

Activate the virtual environment manually, then run `unittest` directly:

```bash
source ~/.venv-panda/bin/activate
python3 -B -m unittest runner.test.<subdir>.<module>
deactivate
```

Examples:

```bash
# Metadata parsing tests
python3 -B -m unittest runner.test.metadata_test.metadata_test

# Macro expansion tests
python3 -B -m unittest runner.test.macro_tests.macro_utils_tests

# Validator chain functional tests
python3 -B -m unittest runner.test.func_test.validators_chain_test

# Config loading tests
python3 -B -m unittest runner.test.config_test.workflow_config_test

# Chapters filtering tests
python3 -B -m unittest runner.test.chapters_test.chapters_test
```

---

## Step 3 — Understand the Test Layout

| Directory | What it tests |
|---|---|
| `chapters_test/` | Chapter-based test filtering |
| `config_test/` | YAML config loading, CLI option parsing, workflow/test-suite config |
| `environment_test/` | Environment variable handling, runner initialisation |
| `func_test/` | End-to-end functional tests (validators chain, expected failures, fail kinds, dependent tests) |
| `generators_test/` | ETS template and function template generators |
| `gtest_test/` | GTest flow and workflow |
| `launches_test/` | CLI option combinations |
| `macro_tests/` | Macro expansion and modifiers |
| `metadata_test/` | `TestMetadata` parsing |
| `roots_test/` | `list-root` and `test-root` resolution |
| `runner_test/` | Test loading and build property parsing |
| `spec_report_test/` | Spec report generation |
| `template_extension_test/` | YAML template extension handling |
| `test_steps_test/` | Step report generation and step execution |
| `updaters_test/` | Updater implementations |
| `validators_tests/` | `AstCheckerValidator`, `ParserValidator`, `NoValidator` |

---

## Step 4 — Interpret Results

- **OK** — all tests passed.
- **FAIL** / **ERROR** — one or more tests failed; the output shows the failing test name and traceback.
- Exit code `0` means success; any non-zero exit code means at least one test failed.

If a test fails after your changes, check:
1. The traceback to identify which assertion failed.
2. Whether the test data in `runner/test/<subdir>/` needs updating.
3. Whether your change broke an existing contract (e.g. config field names, macro syntax).
