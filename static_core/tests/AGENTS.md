# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Test Suite
- **purpose**: Testing infrastructure for runtime, compiler, irtoc, verifier, regression, and performance validation
- **primary language**: C++, Panda Assembly, ETS (.ets)

## About Tests

The **tests/** directory contains several compiler-relevant validation layers:

- **checked/** - main IR/codegen/disasm/events verification layer for compiler changes
- **benchmarks/** - micro-benchmarks
- **vm-benchmarks/** - broader benchmarking harness
- **irtoc-interpreter-tests/** - interpreter/irtoc validation suites
- **verifier-tests/** - verifier-focused coverage
- **regression/** - regression tests
- **tests-u-runner/** and **tests-u-runner-2/** - runner-based workflows used by CI and local repro
- **cts-assembly/** - assembly-based compatibility coverage
- **fuzztest/** - fuzz-based coverage

ETS-specific compiler validation also exists outside this directory under `plugins/ets/tests/compiler/`, `plugins/ets/tests/checked/`,
`plugins/ets/tests/ets_es_checked/`, and `plugins/ets/tests/jitinterface/`.

## Directory Structure

Main File Directories

```
tests/
├── CMakeLists.txt                   # Test build configuration
├── benchmarks/                      # Performance benchmarks
├── checked/                         # Compiler verification tests with checker DSL
├── irtoc-interpreter-tests/         # irtoc interpreter validation
├── regression/                      # Regression tests
├── tests-u-runner/                  # URunner v1
├── tests-u-runner-2/                # URunner v2
├── verifier-tests/                  # Verifier coverage
├── vm-benchmarks/                   # VM/compiler performance suites
└── ...
```

## Build Commands

See @../AGENTS.md

## Running Tests

### Common Compiler-Facing Targets

- `ninja core_checked_tests` - run checked tests
- `ninja irtoc-interpreter-tests` - run irtoc interpreter validation
- compiler gtests are defined under `compiler/` and described in `compiler/AGENTS.md`
- runtime/compiler boundary gtests live in `runtime/tests/stack_walker_test.cpp`, `runtime/tests/c2i_bridge_test.cpp`, and `runtime/tests/i2c_bridge_test.cpp`
- ETS compiler-facing targets include `ets_tests_compiler_optimizer`, `compiler_wrong_boot_context_test`, `ets_interop_js_gtests`, `ets_interop_js_checked_tests`, `ets_run_jit_ets_code_tests`, `ets_run_jit_osr_ets_code_tests`, and `ets_run_aot_ets_code_tests`

### irtoc Change Checklist

- For new or changed `irtoc` fastpaths or intrinsics, cover more than one layer
- Use `ninja irtoc_unit_tests` and `ninja irtoc_compiler_unit_tests` for DSL and backend validation
- Use `ninja irtoc-interpreter-tests` for generated interpreter or entrypoint behavior
- Use `ninja core_checked_tests` plus a focused checked repro such as `tests/checked/osr_irtoc_test.pa` for compiler or runtime interaction
- Run runtime bridge gtests when ABI, stack-frame, or boundary behavior changes
- Add ETS checked or execution coverage when the change is exposed through ETS intrinsics or stdlib APIs
- Do not claim an `irtoc` performance win without comparing it to the managed or C++ baseline on the relevant
  architecture and build mode; see the repo-level policy in `../AGENTS.md`

### Checked Tests

`tests/checked/` is one of the most important tools for compiler work. The checker DSL can validate:

- IR after a chosen pass via `METHOD`, `PASS_AFTER`, `INST`, `INST_NOT`, `IN_BLOCK`
- generated assembly/disassembly via `ASM_METHOD`, `ASM`, `ASM_NOT`
- compiler/runtime events via `EVENT`, `EVENT_NOT`
- JIT, AOT, LLVM AOT, frontend, and bytecode-optimizer workflows via `RUN`, `RUN_PAOC`, `RUN_AOT`, `RUN_LLVM`, `RUN_BCO`, `RUN_FRONTEND`
- advanced workflows also rely on `ASM_INST`, `INST_COUNT`, `IR_COUNT`, `CHECK_GROUP`, `DEFINE_FRONTEND_OPTIONS`, `RUN_PGO_PROF`, and `RUN_PGO_PAOC`

Read `tests/checked/README.md` before adding or debugging compiler checked tests.

Good repro seeds for compiler work:

- `tests/checked/disasm_and_log_demo.pa` - disasm dump + event/log workflow
- `tests/checked/deoptimize_compare.pa` - `DeoptimizeIf` / lowering checks
- `tests/checked/osr.pa` - OSR event validation
- `tests/checked/osr_irtoc_test.pa` - irtoc + OSR interaction

### Important Constraints

- most checked tests are skipped in `Release` unless the test explicitly sets `SUPPORT_RELEASE`
- many checked targets are gated by architecture or sanitizer configuration
- checked OSR cases are arm64-only
- `tests/CMakeLists.txt` also adds arm64-only OSR run variants to broader test suites
- `force_jit: true` expands to `--compiler-hotness-threshold=0 --no-async-jit=true --compiler-enable-jit=true`
- for runtime-side caveats of force-jit, interpreter defaults, or AOT interaction, use `../runtime/AGENTS.md`

### Runner-Based Reproduction

- `tests/tests-u-runner/readme.md` describes how to reproduce CI runs locally
- `tests/tests-u-runner-2/README.md` contains the newer URunner workflow and rerun instructions
- both runners cover configurations such as `INT`, `JIT`, `IRTOC`, `AOT`, and `LLVM`
- both runners can narrow a repro with `--test-file`, `--test-list`, and `--filter`
- for flaky investigation, rerun with `--verbose=all` first and extract the actual `ark` / `ark_aot` command before
  trying to parallelize or stress it
- URunner v1 timeout thread dumps still require `--handle-timeout` and `sudo`
- URunner2 still depends on the environment described in `.urunner.env`; missing mandatory variables are a first-class
  failure mode
- URunner2 repro blocks also include the exact per-step commands plus required environment variables, not only the
  top-level runner invocation
- ETS intrinsic/codegen regressions often live in URunner2 suites such as `compiler_checker_suite` and `ets-es-checked`
- representative ETS URunner2 repros include `plugins/ets/tests/checked_urunner/ets_tests/checks_elimination_osr.ets`,
  `plugins/ets/tests/checked_urunner/ets_tests/escape_deoptimization.ets`, and
  `plugins/ets/tests/checked_urunner/ets_tests/inlining_cache_events.ets`

### Flaky Debug Workflow

Use `../docs/flaky_debugging.md` as the maintained end-to-end workflow. From the tests side, the minimum discipline is:

- start with the exact CI repro line emitted by the current runner or checked target
- keep the same build, panda files, interpreter mode, and AOT/JIT shape
- extract the real `ark` / `ark_aot` command before deeper debugging
- add compiler dumps or GC logging only after the minimized repro still reproduces

### Performance Investigation

- `tests/benchmarks/` and `tests/vm-benchmarks/` are the main core benchmark suites
- `plugins/ets/tests/scripts/micro-benchmarks/README.md` covers ETS micro-benchmark runs across `int`, `jit`, and `aot`
- `runtime/tooling/sampler/docs/cpu_profiler.md` describes the sampling profiler / CPU profiler flow
- use `../compiler/docs/performance_workflows.md` for the compiler-side investigation loop rather than duplicating it here

## Documentation

- `../docs/flaky_debugging.md` - current checked/URunner/high-load/asm-to-IR repro workflow
- `../compiler/docs/performance_workflows.md` - current benchmark and performance workflow

## Code Style

See @../AGENTS.md

## Dependencies

- **Python 3**: For test scripts
- **Ruby**: Required by `tests/checked/checker.rb`
- **es2panda**: Compile .ets to .abc
- **ark**: Run .abc files
- **ark_aot**: AOT and LLVM AOT test workflows
