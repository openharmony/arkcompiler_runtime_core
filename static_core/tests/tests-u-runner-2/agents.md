# Agent: Senior Python developer
# Agents Documentation for URunner-2

## Overview

URunner-2 is a Universal test runner, version 2, designed for Open Harmony testing infrastructure.
This document outlines the various agents and components that work together to provide comprehensive
testing capabilities for the ArkCompiler ecosystem.

## Skills

Skills are step-by-step procedures stored in `.agents/skills/skill/` as Markdown files.
Each skill file starts with a YAML front-matter block (`name`, `description`) followed by the procedure.

| Skill file | Name | When to use |
|---|---|---|
| `.agents/skills/skill/RUN-TESTS.md` | `run-tests` | Running tests for a specified test-suite |
| `.agents/skills/skill/RUN-RUNNER-TESTS.md` | `run-runner-tests` | Running the unit and functional tests of the runner itself (`runner-test.sh`) |
| `.agents/skills/skill/RUN-LINTER.md` | `run-linter` | Running `linter.sh` for code quality checking after any code change |

**How to use a skill:**
1. Identify the task matches a skill (e.g. "run tests for `ets-cts`").
2. Open the corresponding skill file and follow its steps in order.
3. Apply the commands to the current working directory of the project.

---

## Rules & Constraints
- Always use type annotations.
- Minimize using of Any. Any usage of Any type should be documented why it cannot be avoided in the current case.
- Use only libraries from following requirements lists:
  - $ARKCOMPILER_RUNTIME_CORE_PATH/static_core/scripts/dep-lists/requirements-base-python3
  - $ARKCOMPILER_RUNTIME_CORE_PATH/runtime_core/static_core/scripts/dep-lists/requirements-venv-python3
  Where variable **ARKCOMPILER_RUNTIME_CORE_PATH** is declared in the file **.urunner.env**
- Adding of new libraries should be approved.
- Do not comment evident code, keep comments only in complex places.
- Follow PEP8 standard.
- Run **linter.sh** script after changes. Script must be completed with return code is 0.

---

## Core Architecture

### Main Entry Points

- **`main.py`** — Primary entry point for the test runner.
- **`runner.sh`** — Shell script wrapper that activates the virtual environment and launches tests.
- **`linter.sh`** — Script for running `pylint` and `mypy` type checks.

---

## Key Agents

### 1. Runner Core (`runner/suites/`)

#### `RunnerStandardFlow` (`runner/suites/runner_standard_flow.py`)
Main orchestrator for test suite execution.
- Initialises `TestEnv` (config, work dir, coverage, registry).
- Creates the test suite via `suite_registry`.
- Calls `test_suite.process(force_generate)` to get the list of `ITestFlow` objects.
- Manages CPU mask, AOT check, intermediate file cleanup, and coverage HTML generation.
- Inherits `RunnerFileBased`.

#### `TestSuite` (`runner/suites/test_suite.py`)
Manages the full lifecycle of a standard test suite.
- Determines preparation steps (`CopyStep`, `JitStep`, `CustomGeneratorTestPreparationStep`).
- Filters tests by excluded/ignored lists, glob filter, chapters, and group number.
- Creates `ITestFlow` objects via `workflow_registry`.
- Subclass: `GTestSuite` — handles GTest binaries (list tests via `--gtest_list_tests`, ZIP `.abc` files).

#### `TestStandardFlow` (`runner/suites/test_standard_flow.py`)
Implements `ITestFlow` for ETS tests. Registered in `workflow_registry` under key `"ets"`.
- Reads `TestMetadata` from the test file's `/*--- … ---*/` block.
- Implements `do_run()`:
  - Processes dependent files first.
  - Iterates workflow steps (compiler → verifier → AOT → runtime), skipping disabled ones.
  - Per step calls `configure_step_last_call()` (macro expansion, `--module`, `--dump-dynamic-ast`, `ark_options`).
  - Delegates actual execution to `OneStepRunner`.
  - Calls `ValidatorUtils.step_validator()` and records a `StepReport`.
  - Calls `_update_expected()` when `--update-expected` is set.
- Handles negative tests (`is_negative_compile`, `is_negative_runtime`), compile-only tests, and dependent packages.

#### `OneStepRunner` (`runner/suites/one_step_runner.py`)
Executes a single binary step inside a subprocess.
- `run_with_coverage()` — wraps `run_one_step()` with LLVM/LCOV profiling env vars.
- `run_one_step()` — dispatches to `__run()` (standard) or `__run_gtest()` (GTest).
- `__run()` — opens `subprocess.Popen`, reads stdout/stderr, applies timeout, calls `result_validator`, writes stdout/stderr files if configured.
- Detects fail kind (SEGFAULT, ABORT, IRTOC_ASSERT, TIMEOUT, FAIL, PASSED).
- Fills `StepReport` with command line, output, error, return code, validator messages.

#### `GTestFlow` (`runner/suites/gtest_flow.py`)
Specialised flow for GTest framework suites (see `GTestSuite`).

#### `WorkDir` (`runner/suites/work_dir.py`)
Manages the work-directory tree for a test suite run.
- Properties: `root`, `gen`, `intermediate`, `report`, `coverage_dir`, `rerun_failures`.
- On construction clears the previous report directory and, when `--rerun-failed` is set,
  copies `report/failures.txt` to a hidden `.rerun-failures.txt` snapshot so the list is
  preserved before the report directory is wiped.

#### `TestFlowRegistry` (`runner/suites/tests_flow_registry.py`)
Thread-safe runtime registry of test *instances* (distinct from the factory `workflow_registry`).
- Indexes `ITestFlow` objects by `test_id`; ignores duplicate registrations.
- Convenience properties: `valid_tests`, `runtime_tests`.
- Methods: `register(test)`, `find(test_id)`, `filter(predicate)`, `reset()`.

---

### 2. Configuration Management (`runner/options/`)

#### `Config` (`runner/options/config.py`)
Central configuration object merging CLI arguments, YAML workflow and test-suite files, and environment variables.

#### `TestSuiteOptions` (`runner/options/options_test_suite.py`)
Parsed test-suite configuration:
- `suite_name`, `test_root`, `list_roots`, `collections`, `filter`, `extension`, `use_metadata`.
- `validator_class`, `updater_class` — Python class paths loaded dynamically.
- `test_lists` (`TestListsOptions`): `explicit_file`, `explicit_list`, `skip_test_lists`, `rerun_failed`, etc.
- `ets` (`ETSOptions`): `force_generate`.
- `groups` (`GroupsOptions`): `chapters`, `chapters_file`, `quantity`, `number`.

#### `Step` (`runner/options/options_step.py`)
Immutable dataclass representing one workflow step:
- Fields: `name`, `executable_path`, `args`, `timeout`, `step_kind` (`StepKind` enum), `step_filter`,
  `enabled`, `env`, `stdout`, `stderr`, `pre_requirements`, `post_requirements`, `validator_class`, `validator`.
- `StepKind`: `COMPILER`, `VERIFIER`, `AOT`, `RUNTIME`, `GTEST_RUNNER`, `DECLGEN`, `TSC`, `OTHER`.
- `Step.create(name, step_body)` — factory from raw YAML dict.
- `check_pre_requirements()` / `check_post_requirements()` — validate `FileExist`, `FolderExist`, `ParentExistCreate`.

#### `Macros` (`runner/options/macros.py`) / `MacrosModifiers` (`runner/options/macros_modifiers.py`)
YAML macro engine: expands `${yaml.path}` references.
Modifiers: `parent`, `name`, `stem` on path macros; `${test-id}` is a special macro expanded at the last call
before binary execution.

---

### 3. Test Metadata (`runner/suites/test_metadata.py`)

#### `TestMetadata`
Pydantic model extracted from the `/*--- YAML ---*/` block inside `.ets` test files.
- **tags** (`Tags`): `negative`, `compile-only`, `not-a-test`, `no-warmup`.
- `files` — list of dependent file paths.
- `entry_point`, `package`, `arktsconfig`.
- `ark_options`, `es2panda_options` (alias: `flags`).
- `expected_out`, `expected_error` — per-step expected outputs (dict `step_name → list[str]`).
- `negative_steps` — steps expected to fail for this test.
- `test_cli` — extra CLI args appended after `--` in the runtime step.
- Spec-related: `spec`, `desc`, `assertion`, `params`, `name`.
- Test262-specific fields: `description`, `includes`, `features`, `esid`, etc.

#### `Tags`
- Boolean properties: `compile_only`, `negative`, `not_a_test`, `no_warmup`.
- `invalid_tags` — list of unrecognised tag strings (logged as warnings).

---

### 4. Test List Management (`runner/suites/test_lists.py`)

#### `TestLists`
Discovers and filters test list `.txt` files from configured `list_roots`.
- Auto-detects build system (GN / CMake) and reads `args.gn` or `CMakeCache.txt`.
- Derives `sanitizer` (ASAN/TSAN/NONE), `architecture`, `operating_system`, `build_type`, `conf_kind`
  (INT, AOT, AOT_FULL, AOT_PGO, JIT).
- `_get_test_list_pattern(kind)` — builds a regex that matches list filenames carrying the right combination of
  OS/arch/config/interpreter/sanitizer/opt-level/gc-type/build-type suffixes.
- `collect_excluded_test_lists()`, `collect_ignored_test_lists()` — fill `excluded_lists` and `ignored_lists`.
- `resolve_explicit_list()` — honours `--test-list` and `--rerun-failed` options.
- `matched_test_lists()` — glob-pattern filter on discovered list files.

---

### 5. Test Preparation (`runner/suites/preparation_step.py`)

#### `TestPreparationStep` (ABC)
Base class for all preparation steps.
- `transform(force_generated)` → `Generator[Path, None, None]` — yields generated test file paths.
- Helpers: `get_expected_files_for_test()`, `get_d_ets_files()`, `get_metadata_test_files()`.

#### `CopyStep`
Copies test sources (and their `.expected`, `.d.ets`, metadata-declared deps) from `test_source_path` to `test_gen_path`.

#### `JitStep`
Transforms `.ets` files to wrap `main()` in a JIT-repeat loop.
- Uses regex to locate `function main(…)` (sync and async variants).
- Builds a `for (let i = 0; i < N; i++)` wrapper, accumulating the exit code for `int`-returning mains.
- Parallel processing via `multiprocessing.Pool`.

#### `CustomGeneratorTestPreparationStep`
Delegates to a custom `IGenerator` subclass (via `generator-class`) or an external script (via `generator-script`).

---

### 6. Extensions Framework (`runner/extensions/`)

#### Validators (`runner/extensions/validators/`)

| Class | File | Purpose |
|---|---|---|
| `IValidator` | `ivalidator.py` | ABC. Stores `dict[step_name, list[ValidatorFunction]]`. Methods: `add`, `delete`, `get_validator`. |
| `BaseValidator` | `base_validator.py` | Default validator. Registers `check_return_code`, `check_stderr`, `check_output` for every `StepKind`. Handles negative-step expected failures and last-failure checks for ignored tests. |
| `AstCheckerValidator` | `validators/astchecker/astchecker_validator.py` | Overrides compiler step. Parses es2panda JSON AST dump, runs ast-checker test cases, checks return code for positive/negative scenarios. |
| `ParserValidator` | `validators/parser/parser_validator.py` | Validates compiler output against `<test>-expected.txt` files; accepts return codes 0 or 1. |
| `BypassValidator` | `validators/bypass_validator.py` | Always returns `passed=True`. Used on intermediate pipeline steps (e.g. srcdumper AST-dump steps). |
| `Return0Validator` | `validators/return_0_validator.py` | Passes iff return code is 0. |

`ValidatorFunction` signature:
```python
(test: TestStandardFlow, step_fields: StepFields, output: str, error: str, return_code: int) -> ValidationResult
```

#### Generators (`runner/extensions/generators/`)

| Class | File | Purpose |
|---|---|---|
| `IGenerator` | `igenerator.py` | ABC. `generate() -> list[str]`. |
| `EtsTemplatesGenerator` | `generators/ets_cts/ets_templates_generator.py` | DFS traversal of `ets-templates` sources. Expands `Benchmark` (Jinja2-like templates). Copies `.expected` files. Handles `--filter` and `--test-file` with dep resolution. |
| `EtsEsCheckedGenerator` | `generators/ets_es_checked/ets_es_checked_generator.py` | Calls the Ruby/Node.js `generate-es-checked` script to produce ETS tests from TypeScript. |
| `EtsArkJsXgcGenerator` | `generators/ets_arkjs_xgc/ets_arkjs_xgc_generator.py` | Generator for XGC cross-engine tests. |
| `HermesGenerator` | `generators/hermes/hermes_generator.py` | Prepares Hermes JS test sources. |
| `Test262Generator` | `generators/test262/test262_generator.py` | Adapts Test262 ECMAScript tests. |
| `GTestsEtsCompilerGenerator` | `generators/gtests_ets_compiler/gtests_ets_compiler.py` | Generates ETS GTest compiler inputs. |
| `SrcDumperGenerator` | `generators/srcdumper/srcdumper_generator.py` | Generates source-dump tests. |
| `FuncTemplatesGenerator` | `generators/sts_stdlib/func_templates_generator.py` | Generates standard-library functional tests from templates. |
| `StdlibTemplatesGenerator` | `generators/sts_stdlib/stdlib_templates_generator.py` | Renders Jinja2 templates for stdlib tests; splits a rendered output into multiple test files using embedded YAML meta-blocks. |

#### Updaters (`runner/extensions/updaters/`)

| Class | File | Purpose |
|---|---|---|
| `IUpdater` | `iupdater.py` | ABC. `process(test_file, data, use_metadata)`. Activated by `--update-expected`. |
| `ParserUpdater` | `updaters/parser/parser_updater.py` | Updates `<test>-expected.txt` files for parser suites by normalising compiler output. |

#### Flows / Registry

- **`IFlow`** (`flows/iflow.py`) — root ABC for all flow types.
- **`ITestFlow`** (`flows/itest_flow.py`) — ABC with `do_run() -> Self`, `is_valid_test`, `is_negative_compile`.
- **`workflow_registry`** (`flows/test_flow_registry.py`) — `ConfigRegistry[ITestFlow]`. Factory map `name → callable`. `TestStandardFlow` registers under `"ets"`.
- **`suite_registry`** (`suites/test_suite_registry.py`) — `ConfigRegistry[ITestSuite]`. Analogous registry for suite types.
- **`test_suites.py`** (`extensions/suites/test_suites.py`) — wires concrete suite classes into `suite_registry`: `"ets"` → `TestSuite`, `"gtest"` → `GTestSuite`.

#### Steps (`runner/extensions/steps/`)
- **`JitTransformer`** (`steps/jit_transformer.py`) — additional step for JIT transformations.

---

### 7. Reporting (`runner/reports/`)

#### `ReportGenerator` (`runner/reports/report.py`)
Per-test report generation after execution.
- Generates files in `<work_dir>/report/{passed|known|new}/` subdirectories.
- Formats: `HtmlReport`, `MdReport`, `TextReport` (`.log`).
- Verbose filter `all` also generates reports for passed tests.

#### `Report` (ABC) and subclasses
- `HtmlReport` — side-by-side HTML diff of expected vs actual output.
- `MdReport` — Markdown table diff.
- `TextReport` — plain text with command lines to reproduce.

#### `DetailedReport` (`runner/reports/detailed_report.py`)
Folder-level statistics report activated by `--detailed-report`.

#### `SpecReport` (`runner/reports/spec_report.py`)
ArkTS specification coverage report (per chapter/section), activated by `--spec-report`.
Reads spec PDF via `PdfLoader`.

#### `Summary` (`runner/reports/summary.py`)
Aggregates counts of passed / failed / ignored / excluded tests; generates the summary table.

---

### 8. Code Coverage (`runner/code_coverage/`)

#### `CoverageManager` (`runner/code_coverage/coverage_manager.py`)
Central coordinator owning `LcovTool` and `LlvmCovTool`.

#### `LcovTool` (`runner/code_coverage/lcov_tool.py`)
GCOV/LCOV-based coverage: generates `.info` files, `genhtml` HTML report.
Supports per-binary mode with `GCOV_PREFIX` / `GCOV_PREFIX_STRIP`.

#### `LlvmCovTool` (`runner/code_coverage/llvm_cov_tool.py`)
LLVM coverage: merges `.profraw` → `.profdata`, exports to `.info`, generates HTML.
Per-binary: separate profdata list files per component.

#### `CoverageDir` (`runner/code_coverage/coverage_dir.py`)
Manages coverage output directories.

#### `CmdExecutor` (`runner/code_coverage/cmd_executor.py`)
Executes coverage tool commands.

---

### 9. Environment & Initialization (`runner/environment.py`, `runner/init_runner.py`)

#### `RunnerEnv`
Manages environment variables loaded from `.urunner.env` / `.env`.

#### `InitRunner`
Interactive initialization of `.urunner.env` (used by `./runner.sh init`).

---

### 10. Logging (`runner/logger.py`)

#### `Log`
Centralised logger with levels:
- `default` — always shown.
- `short` — shown in `short` and `all` verbose modes.
- `all` — shown only in `all` verbose mode.

---

## Configuration-Based Components

### Workflow files (`cfg/workflows/`)
YAML files describing the sequence of binary steps.
- `from:` key enables step inheritance from a named sub-workflow template.
- `parameters:` — key-value pairs referenced by `${parameters.<key>}` macros inside steps.
- Known workflows: `ark`, `panda-int`, `panda-jit`, `panda-aot`, `panda-aot-pgo`, `checker`,
  `hybrid-ark`, `hybrid-panda-int`, `hybrid-panda-jit`, `panda-vs-node`, `panda-bin-compat`,
  `es2panda-verifier`, `hybrid-es2panda-verifier`, `declgen-ets2ts`, `run-ani-tests`, `run-srcdumper`.

### Test suite files (`cfg/test-suites/`)
YAML files describing test collections, roots, and generation parameters.
- `test-root:` — source of test templates.
- `list-root:` — directory with excluded/ignored `.txt` files.
- `generator-class:` / `generator-script:` — custom generator.
- `validator:` / `updater:` — custom validator/updater class paths.
- `collections:` — named sub-directories (each may override `extension`, `parameters`, etc.).

---

## Agent Interaction / Execution Flow

```
main.py
  └─ RunnerStandardFlow.__init__
       ├─ Config (loads workflow + test-suite YAML, CLI args, env)
       ├─ TestEnv (work_dir, coverage, test_flow_registry)
       ├─ suite_registry.create(workflow_type, test_env)
       │    └─ TestSuite.__init__
       │         ├─ TestLists (build props → sanitizer / arch / conf_kind)
       │         └─ set_preparation_steps → [CopyStep | JitStep | CustomGeneratorStep]
       └─ test_suite.process(force_generate)
            ├─ __prepare_test_files → [CopyStep|JitStep|GeneratorStep].transform()
            ├─ _resolve_test_files (excluded/ignored lists, filter, chapters, groups)
            └─ _create_tests → workflow_registry.create("ets", …) → TestStandardFlow
  └─ RunnerFileBased.run (parallel pool)
       └─ TestStandardFlow.do_run()
            ├─ _continue_after_process_dependent_files
            │    └─ dependent_test.do_run() (recursive)
            └─ per Step:
                 ├─ configure_step_last_call (macro expansion, metadata options)
                 ├─ step.check_pre_requirements()
                 ├─ OneStepRunner.run_with_coverage()
                 │    └─ subprocess.Popen → timeout → result_validator
                 ├─ ValidatorUtils.step_validator → IValidator.get_validator(step)
                 └─ StepReport recorded
  └─ ReportGenerator.generate_reports (HTML / MD / LOG)
  └─ Summary / DetailedReport / SpecReport
  └─ RunnerStandardFlow.create_coverage_html (LCOV / LLVM)
```

---

## Key Patterns and Conventions

### Registry Pattern
Both `workflow_registry` and `suite_registry` are `ConfigRegistry[T]` instances.
Registration uses a `@registry.register("key")` decorator. Creation uses `registry.create("key", …)`.

### Macro Expansion
- **Config-time**: `Macros.correct_macro(value, options)` expands `${yaml.path}` references.
- **Last-call**: `TestStandardFlow.configure_step_last_call()` expands `${test-id}` and remaining macros
  just before subprocess execution. Modifiers `parent`, `name`, `stem` work on path macros.

### Pydantic for Metadata
`TestMetadata` is a `pydantic.BaseModel` with field validators (`@field_validator`) for normalising
tags, expected output, ark/es2panda options, and negative steps.

### Frozen Dataclass for Steps
`Step` is `@dataclass(frozen=True)`. Modifications use `dataclasses.replace(step, …)` to produce
a new instance without mutating the original workflow configuration.

### Parallel Execution
- `RunnerFileBased` runs tests in a `multiprocessing.Pool`.
- `JitStep.transform()` uses a separate `multiprocessing.Pool` for parallel test transformation.
- `CustomGeneratorTestPreparationStep.__generate_by_class()` uses `ThreadPoolExecutor(max_workers=1)`
  to run the generator with UI progress tracking.

---

## Extensibility Points

| Extension Point | How to Extend |
|---|---|
| New test flow | Implement `ITestFlow`, register with `@workflow_registry.register("name")` |
| New test suite | Implement `ITestSuite`, register with `@suite_registry.register("name")` |
| New validator | Subclass `IValidator` (or `BaseValidator`), set `validator:` in test-suite YAML |
| New generator | Subclass `IGenerator`, set `generator-class:` in test-suite YAML |
| New updater | Subclass `IUpdater`, set `updater:` in test-suite YAML |
| New workflow | Add YAML to `cfg/workflows/` |
| New test suite config | Add YAML to `cfg/test-suites/` |
| New report format | Subclass `Report`, register in `ReportGenerator.format_to_report` |
