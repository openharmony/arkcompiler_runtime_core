---
name: run-tests
description: Running tests for a specified test-suite
---

# Skill: Run Tests for a Specified Test Suite

## Overview

This skill covers how to run tests for any test-suite available in URunner-2.
A **test-suite** defines which tests to run (sources, generators, collections).
A **workflow** defines how to run them (which binaries, in which order, with which args).
Both must be specified when invoking the runner.

---

## Step 1 — First-time Setup (if not done yet)

Run the interactive wizard to create `~/.urunner.env`:
```bash
./runner.sh init
```

Set the required variables in `~/.urunner.env`:
```bash
ARKCOMPILER_RUNTIME_CORE_PATH=<path to arkcompiler_runtime_core repo>
ARKCOMPILER_ETS_FRONTEND_PATH=<path to arkcompiler_ets_frontend repo>
PANDA_BUILD=<path to build folder>
WORK_DIR=<path to temporary folder for intermediate files and reports>
```

---

## Step 2 — Choose a Test Suite and a Workflow

### Available test suites (`cfg/test-suites/`)

| Test Suite | Description |
|---|---|
| `ets-cts` | Main ETS conformance tests (language spec coverage) |
| `ets-cts-6.0` | ETS conformance tests for spec version 6.0 |
| `ets-cts-7.0` | ETS conformance tests for spec version 7.0 |
| `ets-cts-ch23` | ETS conformance tests chapters 2–3 |
| `ets-cts-ch23-7.0` | ETS conformance tests chapters 2–3, spec 7.0 |
| `ets-func` | ETS functional tests and stdlib templates |
| `ets-runtime` | ETS runtime-focused tests |
| `ets-sdk` | ETS SDK tests |
| `ets-ts-subset` | ETS/TypeScript subset tests |
| `ets-es-checked` | ETS/ECMAScript checked tests |
| `ets-gc-stress` | ETS GC stress tests |
| `ets-warnings-tests` | ETS compiler warning tests |
| `astchecker` | AST checker tests (JS/TS/ETS parser and compiler) |
| `parser` | Parser tests |
| `declgen-ets2ts-cts` | Declgen ETS→TS conformance tests |
| `declgen-ets2ts-func-tests` | Declgen ETS→TS functional tests |
| `declgen-ets2ts-runtime` | Declgen ETS→TS runtime tests |
| `declgen-ets2ts-sdk` | Declgen ETS→TS SDK tests |
| `declgenparser` | Declgen parser tests |
| `compiler_checker_suite` | Compiler checker suite |
| `hermes` | Hermes JS engine tests |
| `test262` | ECMAScript Test262 suite |
| `ani-tests` | ANI (ArkNative Interface) tests |
| `unit-tests` | Unit tests (GTest binaries) |
| `srcdumper` | Source dumper tests |
| `recheck` | Re-check / rerun previously failing tests |
| `xgc-ts-tests` | XGC TypeScript cross-engine tests |

### Available workflows (`cfg/workflows/`)

| Workflow | Description |
|---|---|
| `panda-int` | Compile with es2panda + verifier, run in Ark interpreter mode |
| `panda-jit` | Compile with es2panda + verifier, run with JIT enabled |
| `panda-aot` | Compile with es2panda + verifier, AOT-compile, run compiled |
| `panda-aot-pgo` | AOT with profile-guided optimisation |
| `panda-all` | Run int + jit + aot in sequence |
| `panda-bin-compat` | Binary compatibility workflow |
| `panda-vs-node` | Compare Ark output vs Node.js output |
| `panda-jit-repeats` | JIT with repeated runs |
| `ark` | Basic Ark runtime workflow |
| `ark-aot` | Ark AOT workflow |
| `ark-aot-pgo` | Ark AOT PGO workflow |
| `checker` | Ruby checker pipeline (Ark + AOT validation) |
| `es2panda-verifier` | Compiler + verifier only (no runtime) |
| `hybrid-ark` | Hybrid Ark runtime workflow |
| `hybrid-panda-int` | Hybrid interpreter workflow |
| `hybrid-panda-jit` | Hybrid JIT workflow |
| `hybrid-es2panda-verifier` | Hybrid compiler + verifier workflow |
| `declgen-ets2ts` | Declgen ETS→TS pipeline |
| `run-ani-tests` | ANI test execution workflow |
| `run-srcdumper` | Source dumper workflow |
| `arkjs-int` | ArkJS interpreter workflow |

---

## Step 3 — Run the Test Suite

### Basic syntax

**Option A — via `runner.sh` (recommended, activates the virtual environment automatically):**

```bash
./runner.sh <workflow-name> <test-suite-name> --force-generate --processes=all [options]
```

**Option B — via the `urunner` package entry point (when installed with `pip install -e`):**

```bash
source ~/.venv-panda/bin/activate
urunner <workflow-name> <test-suite-name> --force-generate --processes=all [options]
deactivate
```

Always include `--force-generate` and `--processes=all` in every run.

### Common run patterns

| Goal                         | Command                                                                                               |
|------------------------------|-------------------------------------------------------------------------------------------------------|
| Run full test suite          | `./runner.sh panda-int ets-cts --force-generate --processes=all`                                      |
| Run single test              | `./runner.sh panda-int ets-cts --force-generate --processes=all --test-file 07.expressions/ne1_0.ets` |
| Filter tests by glob         | `./runner.sh panda-int ets-cts --force-generate --processes=all --filter "07.expressions/25*"`        |
| Verbose output for all tests | `./runner.sh panda-int ets-cts --force-generate --processes=all --verbose all`                        |
| Verbose only for failures    | `./runner.sh panda-int ets-cts --force-generate --processes=all --verbose all --verbose-filter new`   |
| Skip excluded/ignored lists  | `./runner.sh panda-int ets-cts --force-generate --processes=all --skip-test-lists`                    |
| Rerun previous failures      | `./runner.sh panda-int ets-cts --force-generate --processes=all --rerun-failed`                       |
| Run from an explicit list    | `./runner.sh panda-int ets-cts --force-generate --processes=all --test-list my-tests.txt`             |
| Show progress                | `./runner.sh panda-int ets-cts --force-generate --processes=all --show-progress`                      |

> **Tip:** Replace `./runner.sh` with `urunner` (after `pip install -e`) to use the package entry point.

### Suite-specific examples

```bash
# ETS CTS spec 7.0 with default interpreter
./runner.sh panda-int ets-cts-7.0 --force-generate --processes=all
# ETS CTS spec 7.0 with JIT
./runner.sh panda-jit ets-cts-7.0 --force-generate --processes=all
# ETS CTS spec 7.0 with JIT with repeats
./runner.sh panda-jit-repeats ets-cts-7.0 --force-generate --processes=all
# ETS CTS spec 7.0 with AOT
./runner.sh panda-aot ets-cts-7.0 --force-generate --processes=all
# ETS CTS spec 7.0 with AOT PGO
./runner.sh panda-aot-pgo ets-cts-7.0 --force-generate --processes=all

# ETS CTS spec 7.0, only chapter 23
./runner.sh panda-bin-compat ets-cts-ch23-7.0 --force-generate --processes=all

# ETS CTS spec 6.0 with AOT
./runner.sh panda-aot ets-cts-6.0 --force-generate --processes=all

# ETS CTS spec 6.0, only chapter 23
./runner.sh panda-bin-compat ets-cts-ch23-6.0 --force-generate --processes=all

# AST checker suite
./runner.sh es2panda-verifier astchecker --force-generate --processes=all

# Parser suite
./runner.sh es2panda-verifier parser --force-generate --processes=all

# ETS warning suite
./runner.sh es2panda-verifier ets-warning-tests --force-generate --processes=all

# Recheck suite
./runner.sh es2panda-verifier recheck --force-generate --processes=all

# Unit tests (GTest)
./runner.sh panda-all unit-tests --force-generate --processes=all

# Declgen ETS→TS
./runner.sh declgen-ets2ts declgen-ets2ts-cts --force-generate --processes=all
./runner.sh declgen-ets2ts declgen-ets2ts-func-tests --force-generate --processes=all
./runner.sh declgen-ets2ts declgen-ets2ts-func-runtime --force-generate --processes=all
./runner.sh declgen-ets2ts declgen-ets2ts-func-sdk --force-generate --processes=all

# ANI tests
./runner.sh run-ani-tests ani-tests --force-generate --processes=all --filter="*ani_test_*"
./runner.sh run-ani-tests ani-tests --force-generate --processes=all --filter="*ani_verify_test*"

# Source dumper
./runner.sh run-srcdumper srcdumper --force-generate --processes=all

# Compiler checker
./runner.sh checker compiler_checker_suite --force-generate --processes=all

# ETS-TS subset
./runner.sh panda-vs-node ets-ts-subset --force-generate --processes=all
```

---

## Step 4 — Reproduce a CI Failure

CI logs contain a ready-to-use command, for example:
```
To reproduce with URunner run:
./runner.sh panda-int ets-cts --processes all --force-generate \
  --filter=07.expressions/25.equality_expressions/bigint_equality_operators/ne1* \
  --test-file 07.expressions/25.equality_expressions/bigint_equality_operators/ne1_0.ets
```
Copy and run this command locally.

---

## Step 5 — Inspect Results

Reports are written to `<WORK_DIR>/<suite-name>/report/`.

```bash
# Generate HTML report
./runner.sh panda-int ets-cts --report-format html

# Generate Markdown report (default)
./runner.sh panda-int ets-cts --report-format md

# Folder-level statistics
./runner.sh panda-int ets-cts --detailed-report --detailed-report-file report.md

# Execution time breakdown
./runner.sh panda-int ets-cts --time-report
```

Verbose output flags:

| Flag | Effect |
|---|---|
| `--verbose silent` (default) | Show only new failures |
| `--verbose short` | Show status + output for failures |
| `--verbose all` | Show full output for all tests |
| `--verbose-filter new` | Only new failures |
| `--verbose-filter ignored` | New failures + ignored tests |
| `--verbose-filter all` | Every executed test |
