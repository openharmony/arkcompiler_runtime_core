# AGENTS.md

Guidance for AI agents working in `static_core/`. This is the routing hub for the whole subtree: it fixes the
project's shape, terminology, and source-of-truth files, then hands off to the nearest specialized `AGENTS.md`.

## Project Metadata

- **name**: Ark Runtime Core (`static_core`)
- **purpose**: Language-agnostic virtual machine — bytecode execution (interpreter), JIT/AOT/LLVM AOT compilation, and
  garbage collection. Concrete languages are added as plugins; it is **not** tied to a single language.
- **primary language**: C++ (plus Assembly, the irtoc DSL, and code-generation YAML/templates)

## About Static Core

`static_core` is the **core of a multilanguage VM** (historically "Panda Runtime", shipped as **ArkVM**). The core
provides the runtime, the optimizing compiler, the IR-to-code generator (`irtoc`), the bytecode/file toolchain, and the
test infrastructure. Language support is **not** built into the core — each language is a plugin under `plugins/`.

**Terminology (do not conflate):**

- **static_core / ArkVM / Panda Runtime** — the language-agnostic VM in this tree.
- **ArkTS-Sta (a.k.a. ETS)** — the statically-typed ArkTS language. The new changes for ETS VM implementation part
  (compiler extensions, runtime, stdlib, interop) should be **only** under `plugins/ets/`, not in the core directories.
- **Panda Bytecode (`.abc`)** — the portable bytecode the core executes, independent of source language.

Which plugins exist depends on the checkout and on build flags (`-DPANDA_WITH_<LANG>=ON/OFF`); a plugin is auto-enabled
when its `plugins/<lang>/` folder is present. Do not assume a fixed set of languages. See `plugins/AGENTS.md`.

## Source-of-Truth Files

Read the live sources instead of trusting summaries; option names and defaults change:

- `runtime/options.yaml` — runtime (`ark`) CLI options and defaults (JIT, OSR, interpreter selection)
- `compiler/compiler.yaml`, `compiler/tools/paoc/paoc.yaml` — compiler and `ark_aot` option names/defaults
- `README.md` — build, run, and test commands (the canonical command catalog)
- `docs/glossary.md` — authoritative terminology
- `docs/compiler_intro_current.md`, `docs/irtoc.md`, `docs/flaky_debugging.md` — current compiler, irtoc, and
  flaky-debug workflows
- `compiler/docs/performance_workflows.md`, `compiler/docs/aot_pgo.md` — perf and AOT PGO workflows
- `tools/es2panda/AGENTS.md`, `tools/es2panda/README.md` — frontend pipeline and spec-first workflow (`tools/es2panda` is symlink)

## Repository Shape

Use the owning subtree's `AGENTS.md` for detailed guidance rather than this hub:

| Path | Owns |
|---|---|
| `runtime/` | Interpreter, GC, threading, class linking, runtime↔compiler boundary, bridges, deopt, OSR |
| `compiler/` | Language-agnostic optimizing compiler, IR/passes/codegen, AOT/LLVM AOT toolchain |
| `irtoc/` | Interpreter-handler and fastpath generation from the `.irt` DSL |
| `plugins/ets/` | **All ArkTS-Sta (ETS)** VM support: compiler extensions, runtime, stdlib, interop, tests |
| `tools/es2panda/` | (Symlink) Frontend (source → `.abc`) pipeline |
| `assembler/`, `disassembler/`, `libarkfile/`, `abc2program/`, `bytecode_optimizer/` | `.abc` bytecode/file toolchain |
| `libarkbase/` | OS/platform abstraction, base containers, low-level utilities |
| `verification/` | Bytecode verifier |
| `tests/` | Core checked tests, runners (URunner), benchmarks |
| `docs/` | Shared architecture notes owned outside `compiler/` |

## Start Here by Task

Do not stop at this hub. Open the nearest specialized guide first:

- Compiler / JIT / OSR / AOT / codegen → `compiler/AGENTS.md`
- ArkTS-Sta (ETS) VM part → `plugins/ets/AGENTS.md` (then its `compiler/`, `runtime/`, `stdlib/` sub-guides)
- Runtime / GC / bridges / deopt / class linker → `runtime/AGENTS.md`
- Profiling / `.ap` profiles → `runtime/jit/AGENTS.md`
- irtoc / fastpaths / interpreter handlers → `irtoc/AGENTS.md`
- Tests / checked / URunner / benchmarks → `tests/AGENTS.md`

## irtoc Use Policy

Treat `irtoc` as a specialized tool for short, hot, clearly-bounded paths — **not** the default implementation language
for intrinsics or stdlib helpers. Prefer managed code, C++ runtime, a compiler pass, or codegen first. For selection
rules, tradeoffs, and validation expectations, read `irtoc/AGENTS.md`, then the nearest subtree guide.

## Code Style

- Code style enforced by `.clang-format` / `.clang-tidy` and `docs/coding-style.md`.
