# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Ark Compiler Runtime Core Static Core
- **purpose**: Virtual machine runtime designed for OpenHarmony, providing bytecode execution, JIT/AOT compilation, and garbage collection for the ArkTS-Sta programming language
- **primary language**: C++, ArkTS

## About Static Core

This is the **Ark Runtime Core** (`static_core`) for ArkTS-Sta. It contains the runtime, the optimizing compiler, the
IR-to-Code generator (`irtoc`), language plugins, and the test infrastructure used to validate JIT, OSR, AOT, LLVM AOT,
intrinsics, and fast paths.

## Current References

Use the current repository sources:

- `runtime/options.yaml` - runtime CLI defaults such as JIT, OSR, and interpreter selection
- `compiler/compiler.yaml` and `compiler/tools/paoc/paoc.yaml` - compiler and `ark_aot` option names/defaults
- `README.md` - current quick-start commands
- `docs/compiler_intro_current.md` - current compiler-wide notes and caveats
- `docs/irtoc.md` - current irtoc pipeline, modes, validation, and debugging outputs
- `docs/flaky_debugging.md` - current checked/URunner/flaky-debug workflows
- `compiler/docs/performance_workflows.md` and `compiler/docs/aot_pgo.md` - current perf and AOT PGO workflows
- `tools/es2panda/AGENTS.md` and `tools/es2panda/README.md` - frontend pipeline and spec-first workflow

## Start Here for Compiler Work

If the task is compiler-related, do not stay only in this root file. Use the nearest specialized guide:

- `compiler/AGENTS.md` - core compiler guidance
- `plugins/ets/compiler/AGENTS.md` - ETS compiler guidance
- `runtime/AGENTS.md` - runtime/compiler integration guidance
- `runtime/jit/AGENTS.md` - profiling and `.ap` guidance
- `irtoc/AGENTS.md` - irtoc guidance
- `tests/AGENTS.md` - compiler-related test guidance

## irtoc Use Policy

Treat `irtoc` as a specialized tool for short hot paths, not as the default implementation language for intrinsics or
stdlib helpers.

For detailed selection rules, tradeoffs, and validation expectations, use `irtoc/AGENTS.md` first, then the nearest
subtree guide such as `compiler/AGENTS.md`, `runtime/AGENTS.md`, or `tests/AGENTS.md`.

## Repository Shape

Use the owning subtree rather than this root file for detailed guidance:

- `compiler/` - optimizing compiler, pipeline, AOT toolchain, compiler-owned docs
- `plugins/ets/compiler/` - ETS compiler plugin, intrinsics, interop, native-call lowering
- `runtime/` - runtime/compiler boundary, bridges, deopt, OSR, profiling data
- `irtoc/` - interpreter and fastpath generation
- `tests/` - checked tests, runners, and benchmark layers
- `tools/es2panda/` - frontend pipeline and spec-first rules
- `docs/` - shared architecture notes owned outside `compiler/`

For build, smoke-run, and quick-start commands, use `README.md`. Keep this file focused on routing and source-of-truth
decisions rather than duplicating command catalogs.

## Code Style

- 4 spaces indent, 120 character line length
