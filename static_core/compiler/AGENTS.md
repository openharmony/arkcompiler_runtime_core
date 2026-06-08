# ArkTS-Sta Compiler Agent Guide

## 1. Code Map

This AGENTS.md applies to `static_core/compiler/`. This directory turns Ark bytecode into executable code and covers JIT, OSR, AOT, and optional LLVM AOT workflows.

Key areas:

| Path | Responsibility | Risk |
|---|---|---|
| `optimizer/` | Shared compiler body for IR, analyses, optimizations, and codegen | Shared-path changes can affect JIT/OSR/AOT/LLVM AOT together |
| `aot/`, `tools/paoc/`, `tools/aotdump/` | AOT/LLVM AOT products, user entry points, and diagnostics | Input context, product contract, and runtime loading must stay consistent |
| `code_info/` | Metadata for compiled code | Stack walking, exceptions, deopt, OSR, and GC visibility depend on it |
| `tools/aptool/` | Profile file tooling | AOT PGO/JIT profile issues need inspectable evidence |
| `tests/` | Compiler gtests | Proves compiler-local behavior only, not runtime workflow success |

## 2. Knowledge Routing

When a task, path, or term matches the rows below, read the linked knowledge file before planning changes.

| Scenario / Keywords | Read first |
|---|---|
| JIT / hotness / async compile / code install / force-jit | `docs/knowledge/jit-workflow.md` |
| OSR / loop-entry / live state / interpreter-to-compiled | `docs/knowledge/osr-workflow.md` |
| AOT / `ark_aot` / paoc / `.an` / AOT dump / AOT loading | `docs/knowledge/aot-workflow.md` |
| LLVM AOT / LLVM backend / fallback / `paoc-mode=llvm` | `docs/knowledge/llvm-aot-workflow.md` |
| workflow comparison / JIT-OSR-AOT shared boundary / mode-specific regression | `docs/knowledge/compiler-workflows.md` |
| wrong IR/pass/codegen / generated sources / dumps / GraphChecker | `docs/knowledge/compiler-pipeline-debugging.md` |
| existing compiler docs / flags / pass docs / design notes | `docs/README.md` |
| AOT PGO / `.ap` / profile / inline cache / class context | `../runtime/jit/AGENTS.md`, `docs/aot_pgo.md` |
| runtime entrypoint / bridge / stack walker / class linker / deopt | `docs/knowledge/compiled-runtime-boundary.md` |
| ETS plugin intrinsic / ETS-specific IR / native call / JS interop | `../plugins/ets/compiler/AGENTS.md` |
| irtoc / fastpath / nativeplus / interpreter generated code | `../irtoc/AGENTS.md` |
| checked / URunner / benchmark / workflow validation | `../tests/AGENTS.md` |

In the plan, state the matched scenario, documents read, and constraints found.

## 3. Pre-coding Safety Checks

- Must prove pointer safety at compiled/runtime boundaries; avoid UAF, leaks, and null dereference, or cite the ownership/lifetime/invariant that proves non-null.
- Must prove GC visibility for object/reference values live across runtime calls, safepoints, deopt, OSR, bridges, or generated fastpaths through `SaveState`, roots, stack maps, or handles.
- Must treat `CodeInfo`, stack maps, inline info, roots masks, bridge frames, and deopt metadata as correctness data.
- Must identify workflow proof: JIT selection/compile/install/execution, OSR `SaveStateOsr` live-state reconstruction and deoptimized-frame exclusion, AOT context match, and LLVM backend participation when relevant.

## 4. Boundaries

- Do not silently apply assumptions from one workflow to another; shared pipeline, analysis, optimization, or codegen changes must state their affected workflows.
- Do not use compiler gtests alone to prove runtime workflow correctness; JIT/OSR/AOT/LLVM AOT each require matching runtime evidence.
- Do not treat offline `paoc`/`jit`/`osr`/`llvm` mode evidence as proof that runtime selection, install, transition, or fallback behavior is correct.
- Do not weaken diagnostics such as dumps, logs, events, aotdump, or aptool to make tests pass.
- Do not carry raw object pointers across GC, safepoints, runtime calls, deopt, OSR, bridges, or generated fastpaths unless root/`SaveState` visibility is proven.

Ask before:

- changing JIT or OSR trigger policy, hotness defaults, async-vs-sync defaults, install/status ordering, entrypoint publication, or mode-selection/fallback behavior;
- adding user-visible options or long-term dependencies;
- changing the AOT product contract, profile compatibility semantics, runtime entrypoints, or generated-source ownership;
- changing `CodeInfo`, stack-map, roots, bridge, deopt, or OSR metadata layout/ownership/requirements;
- weakening null checks, assertions, `GraphChecker`, matching/rejection/fallback diagnostics, aotdump, or aptool visibility.

## 5. Verification Loop

| Change type | Minimum verification |
|---|---|
| Shared IR/pass/codegen | `compiler_unit_tests` or `compiler_codegen_tests`, plus the affected workflows |
| JIT | Focused JIT run or checked/URunner JIT coverage |
| OSR | Focused OSR run with live-state and transition behavior checked |
| AOT | `ark_aot` product generation, dump/log evidence, and `ark --aot-file` or runner execution |
| LLVM AOT | Evidence for the LLVM AOT path plus fallback/failure paths |

Done evidence must name changed files, documents read, validation run or not run, and remaining workflow risk.
