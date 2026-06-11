# Universal Test Runner, Version 2

## Table of Contents

- [Description](#description)
- [Prerequisites](#prerequisites)
  - [System Requirements](#system-requirements)
  - [Required Repositories](#required-repositories)
  - [Python Environment](#python-environment)
  - [ETS-specific Dependencies](#ets-specific-dependencies)
- [Quick Start](#quick-start)
  - [Step 1 — Initialize the Runner](#step-1--initialize-the-runner)
  - [Step 2 — Run Tests](#step-2--run-tests)
- [Running Tests](#running-tests)
  - [Using runner.sh (Recommended)](#using-runnersh-recommended)
  - [Using main.py Directly](#using-mainpy-directly)
  - [Using the Package Entry Point](#using-the-package-entry-point)
- [Available Workflows](#available-workflows)
- [Available Test Suites](#available-test-suites)
- [Workflow ↔ Test Suite Matching](#workflow--test-suite-matching)
- [Mandatory Options](#mandatory-options)
- [Common Options Reference](#common-options-reference)
  - [Parallelism and Progress](#parallelism-and-progress)
  - [Test Selection Options](#test-selection-options)
  - [Test List Management](#test-list-management)
  - [Verbose and Logging Options](#verbose-and-logging-options)
  - [Report Options](#report-options)
  - [Execution Time Report](#execution-time-report)
  - [ArkTS Specification Coverage Report](#arkts-specification-coverage-report)
  - [Detailed Report](#detailed-report)
- [Test Lists](#test-lists)
  - [Test List Naming Convention](#test-list-naming-convention)
  - [Expected Failure Markup](#expected-failure-markup)
- [Reproducing a CI Test Run](#reproducing-a-ci-test-run)
- [Customized Run](#customized-run)
- [Configuration File Reference](#configuration-file-reference)
  - [Steps](#steps)
  - [Macros](#macros)
  - [Macro Modifiers](#macro-modifiers)
- [Test Development Guide](#test-development-guide)
- [Test Metadata Reference](#test-metadata-reference)
- [Linter and Type Checks](#linter-and-type-checks)
- [Runner Unit Tests](#runner-unit-tests)
- [Srcdumper Test Suite](#srcdumper-test-suite)
- [ETS Dependencies](#ets-dependencies)
- [FAQ](#faq)

---

## Description

Universal Test Runner v2 (URunner2) is a flexible, configurable test execution framework for the
[ArkCompiler Runtime Core](https://gitcode.com/openharmony/arkcompiler_runtime_core) project (Open Harmony).

It orchestrates multi-step test pipelines: compiling ETS/JS sources with `es2panda`, verifying bytecode,
running tests in interpreter, JIT, or AOT mode, collecting results, and generating reports.

---

## Prerequisites

### System Requirements

| Requirement | Details |
|---|---|
| **OS** | Ubuntu (recommended); other Linux distributions may work |
| **Python** | 3.10 or newer |
| **Panda build** | A compiled build of the ArkCompiler Runtime Core |
| **Disk space** | Sufficient space for build artifacts and a work directory for intermediate files |

### Required Repositories

The runner expects the following repositories to be cloned locally:

| Variable | Repository |
|---|---|
| `ARKCOMPILER_RUNTIME_CORE_PATH` | `arkcompiler_runtime_core` — the main runtime repository |
| `ARKCOMPILER_ETS_FRONTEND_PATH` | `arkcompiler_ets_frontend` — the ETS frontend (es2panda) |
| `PANDA_BUILD` | Path to the compiled build directory (output of CMake build) |
| `WORK_DIR` | A writable directory for intermediate files, generated tests, and reports |

### Python Environment

The recommended way to set up the Python environment is to run the install script from the runtime core repository:

```bash
sudo $ARKCOMPILER_RUNTIME_CORE_PATH/static_core/scripts/install-deps-ubuntu -i=test
```

This creates a virtual environment at `~/.venv-panda` with all required Python libraries
(`tqdm`, `dataclasses`, `python-dotenv`, `PyYAML`, `Jinja2`, `pydantic`, etc.).

> **Note:** The runner requires Python 3.10+. Check your version with `python3 --version`.

> **Note:** If you install the runner as a package (see [Using the Package Entry Point](#using-the-package-entry-point)),
> all dependencies listed in `pyproject.toml` are installed automatically via `pip install -e`.

### ETS-specific Dependencies

Some test suites require additional tools:

| Test Suite | Extra Dependencies |
|---|---|
| `ets-es-checked` | `ruby` (installed by default with `-i=dev`), `node`, `ts-node` |
| `ets-ts-subset` | `node`, `ts-node` |

See [ETS Dependencies](#ets-dependencies) for installation instructions.

---

## Quick Start

### Step 1 — Initialize the Runner

Before the first run, create a `.urunner.env` configuration file with your local paths.

**Option A — Interactive initialization (recommended):**

```bash
cd <path-to-tests-u-runner-2>
./runner.sh init
```

This launches an interactive wizard that asks for each required path one by one.
Shell-like input history is preserved — use the **Up arrow** to recall previous values.

**Option B — Non-interactive initialization:**

```bash
./runner.sh init \
  --arkcompiler-runtime-core-path /path/to/arkcompiler_runtime_core \
  --arkcompiler-ets-frontend-path /path/to/arkcompiler_ets_frontend \
  --panda-build /path/to/build \
  --work-dir /path/to/work-dir
```

**Option C — CMake-managed setup**

When URunner is added to the CMake build, the build system generates `${build_dir}/.urunner.env` automatically.
In this mode `runner.sh init` is not required for local runs started from the build directory.

The generated file contains:
```bash
ARKCOMPILER_RUNTIME_CORE_PATH=<runtime_core source root>
ARKCOMPILER_ETS_FRONTEND_PATH=<ets frontend repository root>
PANDA_BUILD=<current build dir>
WORK_DIR=<build dir>/urunner-work
```

For convenience, the build also generates `${build_dir}/runner.sh`, a wrapper around the source runner script with
`STATIC_ROOT_DIR` preset for the current checkout. This wrapper also sets progress to be visible, enables
force-generate and use all processes by default, which are sensible defaults for local development and test
reproduction.
```bash
cd <build_dir>
./runner.sh panda-int ets-runtime # ./runner.sh is ready to use!
```

**Option D — Create the file manually:**

Create `.urunner.env` in the runner directory (or your home directory) with the following content:

```bash
ARKCOMPILER_RUNTIME_CORE_PATH=/path/to/arkcompiler_runtime_core
ARKCOMPILER_ETS_FRONTEND_PATH=/path/to/arkcompiler_ets_frontend
PANDA_BUILD=/path/to/build
WORK_DIR=/path/to/work-dir
```

> **Tip:** The runner searches for `.urunner.env` starting from the current directory upward to the filesystem root,
> then falls back to `~/.urunner.env`. You can keep one global file in your home directory.

### Step 2 — Run Tests

```bash
./runner.sh panda-int ets-cts --show-progress
```

This runs the `ets-cts` test suite using the `panda-int` (interpreter) workflow.

---

## Running Tests

### Using runner.sh (Recommended)

`runner.sh` automatically activates the `~/.venv-panda` virtual environment and runs the tests.

```bash
./runner.sh <workflow-name> <test-suite-name> [options...]
```

**Examples:**

```bash
# Run ETS CTS tests in interpreter mode
./runner.sh panda-int ets-cts --show-progress

# Run with all available CPU cores
./runner.sh panda-int ets-cts --processes all --show-progress

# Run a single test
./runner.sh panda-int ets-cts --test-file 07.expressions/25.equality_expressions/bigint_equality_operators/ne1_0.ets

# Run tests matching a filter pattern
./runner.sh panda-int ets-cts --filter "07.expressions/25.*"

# Run in JIT mode
./runner.sh panda-jit ets-cts --show-progress

# Run in AOT mode
./runner.sh panda-aot ets-cts --show-progress

# Get help for a specific workflow + test suite combination
./runner.sh panda-int ets-cts --help
```

### Using main.py Directly

Activate the virtual environment manually, then run `main.py`:

```bash
source ~/.venv-panda/bin/activate
python3 main.py <workflow-name> <test-suite-name> [options...]
deactivate
```

### Using CMake/Ninja targets

The build may provide local helper targets. E.g.:

- `ur_ets_runtime` - runs `panda-int ets-runtime`
- `ur_ets_astchecker` - runs `es2panda-verifier astchecker`
- `ur_ets_func` - runs `panda-int ets-func`
- `ur_ets_cts` - runs `panda-int ets-cts`

Those targets are also depends on ArkTS Runtime/Frontend components and rebuild them automatically.

Examples:
```bash
ninja -C <build_dir> ur_ets_runtime
```

This setup is intended for local developer workflows. The CMake URunner targets are not intended to be part of the
umbrella `tests` target or CI target chains.

### Using the Package Entry Point

Install the runner as an editable package and use the `urunner` command:

```bash
source ~/.venv-panda/bin/activate
URUNNER2_PATH=<path-to-tests-u-runner-2>
pip install -e ${URUNNER2_PATH}
urunner <workflow-name> <test-suite-name> [options...]
deactivate
```

`pip install -e` installs in editable mode — code changes take effect immediately without reinstalling.

If package dependencies change, upgrade the metadata:

```bash
pip install -e ${URUNNER2_PATH} --upgrade
```

---

## Available Workflows

Workflow config files are located in `cfg/workflows/`. The workflow name is the value of `workflow-name:` in the YAML file.

Some workflows are designed as building blocks that are embedded as steps inside composite workflows.
When run standalone they execute only their own steps — for example, compiling without running the runtime.
Note that some sub-workflows require artifacts produced by a prerequisite workflow: 
for instance, `ark` expects a compiled `.abc` file that `es2panda-verifier` must have produced first.


### General-purpose ETS workflows

| Workflow | Description |
|---|---|
| `panda-int` | Compiles and verifies ETS tests, then runs them in Ark interpreter mode (no JIT) |
| `panda-jit` | Compiles and verifies ETS tests, then runs Ark with JIT enabled |
| `panda-jit-repeats` | Same as `panda-jit`, but wraps `main()` in a loop to warm up the JIT |
| `panda-aot` | Compiles and verifies ETS tests, builds AOT code with `ark_aot`, then runs in AOT mode |
| `panda-aot-pgo` | Compiles and verifies ETS tests, collects profile data, performs PGO AOT compilation, then runs |
| `panda-all` | Runs compile/verify, interpreter, JIT, and AOT execution flows for the same ETS test |
| `panda-bin-compat` | Checks binary compatibility: runs tests with old libraries, then with rebuilt ones. Only for `ets-cts-ch23*` suites |

### ETS sub-workflows

These are designed as building blocks reused as steps inside composite workflows, but can also be run standalone.

| Workflow | Description                                                                                                                                                        |
|---|--------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `es2panda-verifier` | Compiles ETS tests to ABC with es2panda and validates the generated bytecode with verifier. Several test suites run only against the `es2panda-verifier` workflow. |
| `ark` | Runs Ark runtime for the prepared ABC test artifacts                                                                                                               |
| `ark-aot` | Builds AOT output for the prepared ABC test using `ark_aot`                                                                                                        |
| `ark-aot-pgo` | Runs Ark to collect profile data and then performs profile-guided AOT compilation with `ark_aot`                                                                   |

### Hybrid build workflows

For Hybrid builds:

| Workflow | Description |
|---|---|
| `hybrid-panda-int` | Compiles and verifies ETS tests, then runs them in Ark interpreter mode without JIT |
| `hybrid-panda-jit` | Compiles and verifies ETS tests, then runs Ark with JIT enabled |
| `hybrid-es2panda-verifier` | Compiles tests to ABC with es2panda and validates the generated bytecode with verifier |
| `hybrid-ark` | Runs Ark on Hybrid build |

### Specialized workflows

| Workflow | Description |
|---|---|
| `run-srcdumper` | Runs the source-code dumper idempotency pipeline (compile → dump → recompile → compare ASTs). Only for `srcdumper` |
| `declgen-ets2ts` | Runs the declgen_ets2ts tool to generate TypeScript declaration files from ETS sources, then validates them with the TypeScript compiler. Only for `declgen-ets2ts-*` suites |
| `checker` | Runs the Ruby checker pipeline to validate test behavior across Ark runtime and AOT configurations. Only for `compiler_checker_suite` |
| `run-ani-tests` | Compiles `.ets` parts of ANI tests with es2panda, then executes the gtest parts. Only for `ani-tests` |
| `run-gtests` | Compiles ETS test sources and runs the resulting GTest-based binaries. Only for `runtime-gtests`, `runtime-mockstd-gtests` |
| `arkjs-int` | Runs ArkJS tests in interpreter mode (using `es2abc` and `ark_js_vm` binaries). Only for `hermes`, `test262` |
| `panda-vs-node` | Compiles and runs ETS tests with Ark interpreter, then runs equivalent TypeScript with Node.js, and compares the outputs. Only for `ets-ts-subset` |

> **Tip:** Run `./runner.sh <workflow> <suite> --help` to see all parameters specific to a workflow.

---

## Available Test Suites

Test suite config files are located in `cfg/test-suites/`. The suite name is the value of `suite-name:` in the YAML file.

| Suite name | Description |
|---|---|
| `ets-cts` | ArkTS Conformance Test Suite (CTS) — the main ETS language test suite |
| `ets-cts-6.0` | CTS tests for ArkTS specification release 6.0 |
| `ets-cts-7.0` | CTS tests for ArkTS specification release 7.0 |
| `ets-cts-ch23` | CTS tests for ArkTS specification chapter 23 (Binary Compatibility). Only with `panda-bin-compat` |
| `ets-cts-ch23-7.0` | CTS tests for ArkTS specification chapter 23 Binary Compatibility — release 7.0. Only with `panda-bin-compat` |
| `ets-func-tests` | ETS functional tests |
| `ets-runtime` | ETS runtime tests |
| `ets-sdk` | ETS SDK tests |
| `ets-gc-stress` | ETS GC stress tests |
| `ets-es-checked` | ETS tests checked against ES semantics (requires Node.js). No hybrid workflows |
| `ets-ts-subset` | ETS tests that are a subset of TypeScript (requires Node.js). Only with `panda-vs-node` |
| `ets-warnings-tests` | ETS compiler warnings tests. Only with `es2panda-verifier` |
| `hermes` | Hermes JS engine compatibility tests |
| `test262` | ECMAScript Test262 conformance tests |
| `srcdumper` | Source-code dumper idempotency tests. Only with `run-srcdumper` |
| `parser` | Parser test suite. Only with `es2panda-verifier` |
| `astchecker` | AST checker test suite. Only with `es2panda-verifier` |
| `recheck` | Recheck test suite. Only with `es2panda-verifier` |
| `compiler_checker_suite` | Compiler checker test suite. Only with `checker` |
| `declgen-ets2ts-cts` | Declaration generator CTS tests. Only with `declgen-ets2ts` |
| `declgen-ets2ts-func-tests` | Declaration generator functional tests. Only with `declgen-ets2ts` |
| `declgen-ets2ts-runtime` | Declaration generator runtime tests. Only with `declgen-ets2ts` |
| `declgen-ets2ts-sdk` | Declaration generator SDK tests. Only with `declgen-ets2ts` |
| `ani-tests` | ANI (ArkNative Interface) tests. Only with `run-ani-tests` |
| `unit-tests` | Runner unit tests. Compatible with all `panda*` workflows |
| `runtime-gtests` | Runtime GTest tests. Only with `run-gtests` |
| `runtime-mockstd-gtests` | Runtime GTest tests with mock stdlib. Only with `run-gtests` |
| `xgc-ts-tests` | XGC TypeScript tests |

---

## Workflow ↔ Test Suite Matching

The table below shows which workflows are accepted by each group of test suites.

| Test suite(s) | Accepted workflows |
|---|---|
| `ets-cts`, `ets-cts-6.0`, `ets-cts-7.0`, `ets-func-tests`, `ets-runtime`, `ets-sdk`, `ets-gc-stress`, `unit-tests` | `ark`, `ark-aot`, `ark-aot-pgo`, `es2panda-verifier`, `hybrid-ark`, `hybrid-es2panda-verifier`, `hybrid-panda-int`, `hybrid-panda-jit`, `panda-all`, `panda-aot`, `panda-aot-pgo`, `panda-int`, `panda-jit`, `panda-jit-repeats` |
| `ets-es-checked` | `ark`, `ark-aot`, `ark-aot-pgo`, `es2panda-verifier`, `panda-all`, `panda-aot`, `panda-aot-pgo`, `panda-int`, `panda-jit`, `panda-jit-repeats` |
| `ets-cts-ch23`, `ets-cts-ch23-7.0` | `panda-bin-compat` |
| `ets-ts-subset` | `panda-vs-node` |
| `ets-warnings-tests`, `parser`, `astchecker`, `recheck` | `es2panda-verifier` |
| `srcdumper` | `run-srcdumper` |
| `declgen-ets2ts-cts`, `declgen-ets2ts-func-tests`, `declgen-ets2ts-runtime`, `declgen-ets2ts-sdk` | `declgen-ets2ts` |
| `compiler_checker_suite` | `checker` |
| `ani-tests` | `run-ani-tests` |
| `runtime-gtests`, `runtime-mockstd-gtests` | `run-gtests` |
| `hermes`, `test262` | `arkjs-int` |

> **Tip:** The runner validates the workflow–suite combination at startup and prints an error if they are incompatible.

> **Tip:** Run `./runner.sh <workflow> <suite> --help` to see all parameters specific to a workflow.


---

## Mandatory Options

Every runner invocation requires exactly two positional arguments:

```
./runner.sh <workflow-name> <test-suite-name> [options...]
```

- `<workflow-name>` — must match a `workflow-name:` value in one of the files under `cfg/workflows/`
- `<test-suite-name>` — must match a `suite-name:` value in one of the files under `cfg/test-suites/`

---

## Common Options Reference

### Parallelism and Progress

| Option | Default | Description |
|---|---|---|
| `--processes N` / `-j N` | `1` | Number of parallel worker processes. Use `all` to use all CPU cores |
| `--show-progress` | off | Display a progress bar during test execution |
| `--continue-if-failed` | off | Continue executing subsequent workflow steps even if a previous step failed |
| `--force-generate` | Regenerate ETS test cases from templates (use after changing test sources or templates) |

### Test Selection Options

| Option                 | Description                                                                                                                                                                                                  |
|------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `--test-file TEST_ID`  | Run only the single specified test file by test-id (path relative to `test_root`)                                                                                                                            |
| `--test-list FILE`     | Run only the tests listed in the given file                                                                                                                                                                  |
| `--filter GLOG`        | Run only tests whose path matches the given glob expression                                                                                                                                                  |
| `--rerun-failed`       | Rerun only tests that failed in the previous run (reads `failures.txt` from the work directory)                                                                                                              |
| `--chapters CHAPTERS`  | Run only tests belonging to the specified chapters (semicolon-separated list, e.g. `chapter05;chapter07`). Chapter names must match chapter names described in `chapters.yaml` (managed by the option `--chapters-file`) |
| `--chapters-file FILE` | Path to a YAML file describing chapter names. Defaults to `chapters.yaml` located alongside the test lists                                                                                                   |
| `--groups N`           | Divide all tests into `N` equal groups. Default is `1` (all tests in one group). Use together with `--group-number` to run a specific slice of tests                                                        |
| `--group-number M`     | Run only the M-th group (1-based) when `--groups` is set. Default is `1` (first group). If the value exceeds the total number of groups, the last group is used                                             |

### Test List Management

| Option | Description |
|---|---|
| `--skip-test-lists` | Ignore all excluded/ignored test lists; run all tests and report all failures |
| `--exclude-ignored-tests` | Treat all ignored test lists as excluded (tests are skipped, not run) |
| `--exclude-ignored-test-lists LIST` | Treat specific ignored test list as excluded (can be repeated) |
| `--update-excluded` | After the run, regenerate the excluded test list with all currently failing tests |
| `--update-expected` | Regenerate expected output files for test suites that support updaters |
| `--test-list-arch ARCH` | Load architecture-specific test lists. One of: `AMD64`, `AMD32`, `ARM64`, `ARM32` |
| `--test-list-san SAN` | Load sanitizer-specific test lists. One of: `none`, `asan`, `tsan` |
| `--test-list-os OS` | Load OS-specific test lists. One of: `LIN`, `WIN`, `MAC` |
| `--test-list-build BUILD` | Load build-type-specific test lists. One of: `DEBUG`, `RELEASE`, `FAST-VERIFY` |

### Verbose and Logging Options

| Option | Values | Description |
|---|---|---|
| `--verbose` / `-v` | `all`, `short`, `silent` | Verbosity level. `all` = most detailed; `short` = status + output; `silent` = only new failures (default) |
| `--verbose-filter` | `all`, `ignored`, `new` | Which tests to print output for. `all` = every test; `ignored` = new failures + ignored list tests; `new` = only new failures (default) |

### Execution Time Report

Collects statistics on how long individual tests take to run. Tests are grouped by execution time.

| Option                 | Description                              |
|------------------------|------------------------------------------|
| `--enable-time-report` | Log execution test time. |
| `--time-edges RUNNER.TIME-EDGES`        | defines grouping boundaries in seconds. For example, `1,5,10` creates four groups: <1s, 1–5s, 5–10s, ≥10s. |

After the run:
- A short summary is printed to the console
- A full report is saved to `<work-dir>/report/<test-suite>-time_report-<timestamp>.txt`

### ArkTS Specification Coverage Report

Generates a report showing test statistics per specification chapter or section.

| Option | Config key | Description |
|---|---|---|
| `--spec-report` | `report.spec-report: True` | Enable the spec coverage report (generates two files) |
| `--spec-report-file FILE` | `report.spec-report-file: FILE` | Path for the Markdown-formatted report |
| `--spec-report-yaml FILE` | `report.spec-report-yaml: FILE` | Path for the YAML-formatted report |
| `--spec-file FILE` | `report.spec-file: FILE` | Path to the ArkTS specification PDF file |

### Detailed Report

Shows test statistics broken down by folder.

| Option | Description |
|---|---|
| `--detailed-report` | Enable the detailed report |
| `--detailed-report-file FILE` | Path where the detailed report should be saved |

A report is saved to `<work-dir>/report/<test-suite>-<detailed-report-file>`. The file format is Markdown.

---

## Test Lists

The runner supports two kinds of test lists:

| Kind | Behavior |
|---|---|
| **excluded** | Tests are not run at all ("Don't try to run") |
| **ignored** | Tests are run, but failures are not counted as new failures. Also called "known failures list" (KFL) |

### Test List Naming Convention

The test list filename pattern (generated by `_get_test_list_pattern` in `runner/suites/test_lists.py`):

```
<suite-name>[-<info>]-<kind>[-<OS>][-<arch>][-<config>][-<interpreter>][-<sanitizer>][-OL<opt-level>][-DI][-FULLASTV][-SIMULTANEOUS][-REPEATS][-<gc-type>][-<build-type>].txt
```

All segments except `kind` are optional — if absent the list applies to all matching configurations. Segments must appear **in the order shown above**.

| Segment | Values                                                               | Notes                                                                                                                  |
|---|----------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------|
| `kind` | `excluded`, `ignored`                                                | Required                                                                                                               |
| `OS` | `LIN`, `WIN`, `MAC`                                                  | Applies to all OSes if absent                                                                                          |
| `arch` | `ARM32`, `ARM64`, `AMD32`, `AMD64`                                   | Applies to all architectures if absent                                                                                 |
| `config` | `INT`, `AOT`, `AOT-FULL`, `AOT-PGO`, `JIT`, `QUICK`                  | Applies to all configurations if absent                                                                                |
| `interpreter` | Any interpreter-type value, e.g. `CPP`                               | Applies to default interpreter type if absent                                                                          |
| `sanitizer` | `ASAN`, `TSAN`                                                       | Included only when sanitizers are enabled in the build. The 'ASAN' mark includes both 'ASAN' and 'UBSAN' sanitizers. |
| `OL<n>` | `OL0`, `OL2`, etc.                                                   | Included only when `--opt-level` is set                                                                                |
| `DI` | literal `DI`                                                         | Included only when `--debug-info` es2panda arg is set                                                                  |
| `FULLASTV` | literal `FULLASTV`                                                   | Included only when `--verifier-all-checks` es2panda arg is set                                                         |
| `SIMULTANEOUS` | literal `SIMULTANEOUS`                                               | Included only when `--simultaneous=true` es2panda arg is set                                                           |
| `REPEATS` | literal `REPEATS`                                                    | Included only for JIT config when `--jit-repeats` > 1                                                                  |
| `gc-type` | Current GC type value, e.g. `G1-GC`                                  | Always included; defaults to `G1-GC`                                                                                   |
| `build-type` | `DEBUG`, `RELEASE`, `FAST-VERIFY`, `RELWITHDEBINFO`, `DEBUGDETAILED` | Derived from the cmake build type                                                                                      |

**Examples:**

| File name | Meaning |
|---|---|
| `ets-cts-ignored.txt` | Ignored tests for `ets-cts`, any configuration |
| `ets-cts-excluded.txt` | Excluded tests for `ets-cts`, any configuration |
| `test262-flaky-ignored-JIT.txt` | Ignored flaky tests for `test262`, JIT configuration only |
| `hermes-excluded.txt` | Excluded tests for `hermes`, any configuration |
| `ets-func-tests-ignored.txt` | Ignored tests for `ets-func-tests`, any configuration |
| `ets-cts-FastVerify-ignored-OL2.txt` | Ignored tests for `ets-cts`, opt-level 2 only |

Test entries in list files are paths relative to `test_root`, the collection name can be omitted, but it's not recommended:

```
array-large.js
built-ins/Date/UTC/fp-evaluation-order.js
tests/stdlib/std/math/sqrt-negative-01.ets
```

The same format is used for `--test-file`.

By default, test lists are loaded automatically from `LIST_ROOT`. Architecture and sanitizer options are read from the cmake cache. 
Values set through command line options are ignored.

> **Note:** Test list options control which lists are loaded. They do not affect where the runner itself or the test binaries are executed.

### Expected Failure Markup

An ignored test entry can include an expected failure pattern. If the actual failure no longer matches the pattern, the test is reclassified as a **NEW FAILURE**:

```text
#26512 @@Failure: SyntaxError: Unexpected token *@@
02.lexical_elements/09.literals/08.regex_literal/regex_anchor_boundaries.ets
```

---

## Reproducing a CI Test Run

When a test fails on CI, the log includes a reproduction command:

```
To reproduce with URunner run:
(min parameters set): ${runner_path}/runner.sh panda-int ets-cts --processes all --force-generate --show-progress \
  --filter=07.expressions/25.equality_expressions/bigint_equality_operators/ne1* \
  --test-file 07.expressions/25.equality_expressions/bigint_equality_operators/ne1_0.ets

(full parameters set): ${runner_path}/runner.sh panda-int ets-cts --extension ets --load-runtimes ets \
  --work-dir ${path to workflow folder} --build ${build path} --opt-level 2 \
  --es2panda-timeout 30 --verifier-timeout 30 --enable-es2panda True --enable-verifier True \
  --ark-timeout 180 --gc-type g1-gc --full-gc-bombing-frequency 0 \
  --heap-verifier fail_on_verification --is-panda True \
  --processes all --show-progress True --force-generate True \
  --test-file 07.expressions/25.equality_expressions/bigint_equality_operators/ne1_0.ets
```

Copy the parameters from the CI log and run locally. The failure output also includes the required environment variables and the exact per-step commands executed by the runner.

---

## Customized Run

To run an arbitrary set of tests with custom configuration:

1. **Create a workflow config** in `cfg/workflows/` with the required parameters and steps (optional if an existing workflow fits).
2. **Create a test suite config** in `cfg/test-suites/` with the required paths to test templates and ignore lists (optional if an existing suite fits).
3. **Run** with your configs:

```bash
./runner.sh <your_workflow_name> <your_suite_name> [options...]
```

---

## Configuration File Reference

### Steps

A workflow YAML file defines a sequence of steps. Each step specifies a binary to execute:

```yaml
steps:
    es2panda:                                          # step name (user-defined)
        executable-path: ${parameters.build}/bin/es2panda
        timeout: ${parameters.es2panda-timeout}        # in seconds
        enabled: ${parameters.enable-es2panda}         # true/false — enables or disables this step
        step-type: compiler
        stdout: "${parameters.work-dir}/gen/${test-id}.output.txt"
        stderr: "${parameters.work-dir}/gen/${test-id}.error.txt"
        validator: runner.path.to.CustomValidator      # optional: Python class path (inherits IValidator)
        pre-reqs:                                      # checks before the binary runs
            - req: FileExist
              value: "${parameters.work-dir}/gen/simple_co.ets"
            - req: FolderExist
              value: "${parameters.work-dir}/gen"
            - req: ParentExistCreate                   # creates parent directory if missing
              value: "${parameters.output-file}"
        post-reqs:                                     # checks after the binary runs
            - req: FileExist
              value: "${parameters.work-dir}/gen/simple_co.ets"
        args:
            - "${parameters.es2panda-full-args}"
            - "${parameters.work-dir}/gen/${test-id}"
```

A step can also be imported from another workflow config using `from:`:

```yaml
steps:
    compile:
        from: es2panda-verifier          # imports steps from cfg/workflows/es2panda-verifier.yaml
        build: ${parameters.build}
        work-dir: ${parameters.work-dir}
```

### Macros

Values in workflow and test-suite YAML files can contain macros in the form `${...}`.
The macro name is the YAML path to the desired property.

**Special macro:**

| Macro | Expands to |
|---|---|
| `${test-id}` | The test file path relative to `TEST_ROOT`. Expanded just before the binary is executed |

**Environment variable macros:**

Any environment variable defined in `.urunner.env` can be referenced directly:

```yaml
build: ${PANDA_BUILD}
test-root: ${ARKCOMPILER_RUNTIME_CORE_PATH}/static_core/plugins/ets/tests/ets-templates
```

### Macro Modifiers

| Modifier | Description | Input example | Output example |
|---|---|---|---|
| *(none)* | Expands macro as-is | `${test-id}` | `/path/to/test.ets` |
| `parent` | Extracts parent directory | `${test-id:parent}` | `/path/to` |
| `name` | Extracts filename with extension | `${test-id:name}` | `test.ets` |
| `stem` | Extracts filename without extension | `${test-id:stem}` | `test` |

---

## Test Development Guide

For information on how to write and annotate ETS test files — including YAML front-matter syntax,
supported metadata fields, expected output/error validation, and ignored-list markup —
see **[test-development.md](test-development.md)**.

---

## Linter and Type Checks

Run linters and type checks with:

```bash
./linter.sh
```

Or via `tox`:

```bash
source ~/.venv-panda/bin/activate
tox run -e linters
deactivate
```

This runs:
- `pylint` (settings in `.pylintrc`)
- `mypy` (settings in `mypy.ini`)
- `flake8`
- `ruff`

---

## Runner Unit Tests

Run unit tests (with coverage report) via `tox`:

```bash
source ~/.venv-panda/bin/activate
tox run -e unittests
deactivate
```

## Srcdumper Test Suite

The `srcdumper` test suite verifies that the **es2panda source-code dumper** is idempotent: it compiles an ETS source
file, dumps the reconstructed source, recompiles the dump, and then compares the two resulting ASTs.

### How it works

The workflow `run-srcdumper` executes the following pipeline for every test:

1. **`compile-original-with-dump-src`** – compiles the original `.ets` file with `--dump-ets-src-after-phases` and
   saves the reconstructed source to `intermediate/dump/<test-id>`.
2. **`strip-first-line`** – runs `cfg/scripts/strip-dump.sh` to remove the es2panda phase-name header line (and any
   trailing fatal-error lines) from the dump file.
3. **`compile-original-with-dump-ast`** – compiles the *original* source with `--dump-after-phases` and saves the
   JSON AST to `intermediate/ast/<test-id>.ast`.
4. **`compile-dumped-with-dump-ast`** – compiles the *dumped* source with `--dump-after-phases` and saves the JSON
   AST to `intermediate/dump-ast/<test-id>.dumped.ast`.
5. **`compare-dumps`** – runs `cfg/scripts/compare-dumps.sh` which calls `AstComparator` to compare the two AST
   files.  The step fails if the ASTs differ.

Steps 3 and 4 use `BypassValidator` so that a non-zero compiler exit code does not immediately fail the test; only the
final AST comparison step determines pass/fail.

### Running the srcdumper suite

```bash
./runner.sh run-srcdumper srcdumper [options]
```

### Test-suite configuration (`cfg/test-suites/srcdumper.yaml`)

| Property | Value |
|---|---|
| `suite-name` | `srcdumper` |
| `list-root` | `${ARKCOMPILER_ETS_FRONTEND_PATH}/ets2panda/test/test-lists/srcdumper` |
| `test-root` | `${ARKCOMPILER_ETS_FRONTEND_PATH}/ets2panda/test` |
| `generator-class` | `runner.extensions.generators.srcdumper.srcdumper_generator.SrcdumperGenerator` |
| Collections | `compiler/ets`, `parser/ets`, `runtime/ets`, `ast`, `srcdump` |

Tests that contain an error-expectation annotation (`/*@@…*/` or `/*@@?…*/`) are **automatically skipped** by the
generator because they are expected to fail compilation and therefore cannot produce a valid dump.

### Generator: `SrcdumperGenerator`

**Location:** `runner/extensions/generators/srcdumper/srcdumper_generator.py`

`SrcdumperGenerator` extends `IGenerator` and prepares the test corpus for the srcdumper workflow:

- Scans each configured collection for `*.ets` files.
- **Skips** files that contain an error-expectation annotation (`/*@@…*/` / `/*@@?…*/`).
- Resolves transitive file dependencies declared in test metadata (`files:` field) and copies them alongside the
  primary test file into the generation target directory.

### Validator: `BypassValidator`

**Location:** `runner/extensions/validators/bypass_validator.py`

A pass-through validator that always returns `passed=True` regardless of the step's exit code or output.
Use it on intermediate pipeline steps whose failure should not immediately abort the test (e.g. the AST-dump
compilation steps in the srcdumper workflow).

```yaml
validator: runner.extensions.validators.bypass_validator.BypassValidator
```

#### `AstComparator` (srcdumper validator)

**Location:** `runner/extensions/validators/srcdumper/ast_comparator.py`

Compares two es2panda JSON AST dump files for semantic equivalence.  Before comparison the ASTs are
*normalised*:

| Normalisation step | Description |
|---|---|
| `remove_loc_nodes` | Strips all `loc` keys (source-location info). |
| `remove_empty_statements` | Removes `EmptyStatement` nodes. |
| `replace_null_literals` | Unifies `NullLiteral` and `ETSNullType` into a single sentinel value. |
| `flatten_similar_nested_nodes` | Flattens nested `BlockStatement`/`ETSUnionType` nodes. |
| `remove_duplicate_undefined_types` | Keeps only one `ETSUndefinedType` per list. |

`AstComparator` can also be invoked as a standalone CLI tool:

```bash
python3 -m runner.extensions.validators.srcdumper.ast_comparator <dump1.ast> <dump2.ast>
# exits 0 if equal, 1 if different
```

The companion script `cfg/scripts/strip-dump.sh` (wrapping `strip_dump.py`) removes the phase-name header line
and any trailing fatal-error lines from a raw es2panda dump file before it is fed back into the compiler.

## ETS dependencies

```bash
source ~/.venv-panda/bin/activate
python -m coverage run -m unittest discover -s runner/test -p "*_test*.py"
python -m coverage report -m
deactivate
```

---

## ETS Dependencies

### ets-es-checked test suite

Requires:
- `ruby` (installed by default with `$PROJECT/static_core/scripts/install-deps-ubuntu -i=dev`)
- `node` and `ts-node`

### ets-ts-subset test suite

Requires:
- `node` and `ts-node`

### Node.js Installation

```bash
sudo apt-get -y install npm
sudo npm install -g n
sudo n install 22
cd $PROJECT/static_core/tests/tests-u-runner-2/runner/extensions/generators/ets_es_checked/generate-es-checked
npm install
```

> **Tip:** If you use `nvm`, you can set `NODE_PATH` in `.urunner.env` to point to your Node.js installation:
> ```bash
> NODE_PATH=/home/user/.nvm/versions/node/v22.21.0
> ```

---

## FAQ

### Q: The runner fails with "Build path is not set" or "Path does not exist"

**A:** The `.urunner.env` file is missing or contains incorrect paths. Run `./runner.sh init` to create or
recreate it interactively. Make sure `PANDA_BUILD` points to an existing compiled build directory and
`ARKCOMPILER_RUNTIME_CORE_PATH`/`ARKCOMPILER_ETS_FRONTEND_PATH` point to the cloned repositories roots.

---

### Q: Where is `.urunner.env` searched for?

**A:** The runner searches for `.urunner.env` starting from the current working directory, going up the directory
tree to the filesystem root. If not found, it falls back to `~/.urunner.env` (home directory).
You can keep a single global file in your home directory and it will be picked up from any location.

---

### Q: How do I run only one test?

**A:** Use `--test-file` with the path relative to the test suite's `test_root`:

```bash
./runner.sh panda-int ets-cts --test-file 07.expressions/25.equality_expressions/bigint_equality_operators/ne1_0.ets
```

---

### Q: How do I run tests matching a pattern?

**A:** Use `--filter` with a glob expression. The pattern is matched against the test path relative to `test_root`.

```bash
# All tests under chapter 07, section 25
./runner.sh panda-int ets-cts --filter "07.expressions/25.*"

# All tests whose name starts with "ne1" anywhere in the tree
./runner.sh panda-int ets-cts --filter "**/ne1*"

# All tests in any subdirectory of chapter 09 that contain "class" in the name
./runner.sh panda-int ets-cts --filter "09.*/**/*class*"

# All tests under chapter 12 recursively
./runner.sh panda-int ets-cts --filter "12.error_handling/**"
```

---

### Q: How do I speed up the test run?

**A:** Use `--processes all` to run tests in parallel across all available CPU cores:

```bash
./runner.sh panda-int ets-cts --processes all --show-progress
```
>Be careful using `all` in a virtual environment with limited resources. It uses the `nproc` command, which may ignore those limits. 

---

### Q: Tests are failing but I don't see any output. How do I get more details?

**A:** Use `--verbose` to increase output verbosity:

```bash
# Show status for every test, but output for new failures only
./runner.sh panda-int ets-cts --verbose=short

# Show status for every test, but output for new and known (ignored) failures
./runner.sh panda-int ets-cts --verbose=short --verbose-filter=ignored

# Show everything for all tests. Be careful with this option, it may generate a lot of output.
./runner.sh panda-int ets-cts --verbose=all --verbose-filter=all
```

---

### Q: How do I rerun only the tests that failed in the last run?

**A:** Use `--rerun-failed`. The runner reads `failures.txt` from the work directory of the previous run:

```bash
./runner.sh panda-int ets-cts --rerun-failed
```

---

### Q: What is the difference between "excluded" and "ignored" test lists?

**A:**
- **Excluded** tests are not run at all. They are completely skipped.
- **Ignored** tests are run, but their failures are not counted as new failures. 
They appear in the report as "known failures". This is useful for tracking tests that are known to be broken (e.g., open bugs).

---

### Q: A test is in the ignored list but the failure message changed. What happens?

**A:** If an ignored test entry has an `@@Failure: <pattern>@@` marker and the actual failure no longer matches
the pattern, the runner reclassifies it as a **NEW FAILURE**. This helps detect when a known failure changes
its nature (e.g., a different error message), which may indicate a regression or a fix that needs the list updated.

---

### Q: How do I add a test to the ignored list with an expected failure?

**A:** Add a comment line with the failure marker immediately before the test entry in the `.txt` test list file:

```text
#12345 issue description @@Failure: SyntaxError: Unexpected token \*@@
02.lexical_elements/09.literals/08.regex_literal/regex_anchor_boundaries.ets
```

---

### Q: How do I regenerate test cases from templates?

**A:** Use `--force-generate`. This is required after changing test source templates:

```bash
./runner.sh panda-int ets-cts --force-generate
```

---

### Q: How do I see what parameters a specific workflow or test suite accepts?

**A:** Run with `--help` after specifying the workflow and test suite:

```bash
./runner.sh panda-int ets-cts --help
```

This shows three sections:
1. Parameters of the specified workflow (`panda-int`)
2. Parameters of the specified test suite (`ets-cts`)
3. Common runner parameters

---

### Q: How do I reproduce a CI failure locally?

**A:** Copy the reproduction command from the CI log. It looks like:

```
To reproduce with URunner run:
(min parameters set): ${runner_path}/runner.sh panda-int ets-cts --processes all ...
```

Replace `${runner_path}` with the path to your `tests-u-runner-2` directory and run the command.
The CI log also includes the exact environment variables and per-step commands used.

---

### Q: How do I create a custom workflow or test suite?

**A:**
1. Copy an existing YAML file from `cfg/workflows/` or `cfg/test-suites/` as a starting point.
2. Edit the `workflow-name:` or `suite-name:` field to a unique name.
3. Adjust parameters, steps, and paths as needed.
4. Run with your new names:

```bash
./runner.sh my-workflow my-suite
```

---

### Q: The `ets-es-checked` or `ets-ts-subset` suite fails with a Node.js error

**A:** These suites require Node.js v22 and `ts-node`. Install them:

```bash
sudo apt-get -y install npm
sudo npm install -g n
sudo n install 22
cd $PROJECT/static_core/tests/tests-u-runner-2/runner/extensions/generators/ets_es_checked/generate-es-checked
npm install
```

If you use `nvm`, also set `NODE_PATH` in your `.urunner.env`.

---

### Q: How do I run the runner's own unit tests?

**A:**

```bash
source ~/.venv-panda/bin/activate
tox run -e unittests
deactivate
```

---

### Q: How do I check what test lists are being loaded?

**A:** Use `--verbose=short` to see what test lists are loaded. Use `--verbose=all` to see what tests in test lists are duplicated or absent on the file system.

---

### Q: Can I use the runner without the `~/.venv-panda` virtual environment?

**A:** Yes. You can install the runner as a package into any Python 3.10+ virtual environment:

```bash
python3 -m venv my-env
source my-env/bin/activate
pip install -e /path/to/tests-u-runner-2
urunner panda-int ets-cts --show-progress
```

---

### Q: What does `--skip-test-lists` do?

**A:** It disables all excluded and ignored test lists. Every discovered test is run, and every failure is
reported as a new failure. This is useful for auditing the full test suite state or when you want to see
all failures regardless of known-failure lists.

---

### Q: How do I update the excluded test list after fixing tests?

**A:** Run with `--update-excluded`. After the run, the excluded list is regenerated to contain only the
currently failing tests:

```bash
./runner.sh panda-int ets-cts --update-excluded
```

---

### Q: What is the work directory and what does it contain?

**A:** The work directory (`WORK_DIR`) is where the runner stores all intermediate and output files:

| Path                                             | Contents                                                          |
|--------------------------------------------------|-------------------------------------------------------------------|
| `<WORK_DIR>/<suite-name>/gen/`                   | Generated and copied test and dependent files                     |
| `<WORK_DIR>/<suite-name>/intermediate/`          | Compiled artifacts (`.abc` files, etc.)                           |
| `<WORK_DIR>/<suite-name>/report/`                | Test reports (`.md`, `.html`, `.log`)                             |
| `<WORK_DIR>/<suite-name>/report/failures.txt`    | List of failed tests from the last run (used by `--rerun-failed`) |
| `<WORK_DIR>/<suite-name>/report/time_report.txt` | Execution time report (when `--time-report` is used)              |

> **Warning:** The work directory can grow large. Clean it periodically or use a dedicated partition.
