# Compiler AOT Agent Guide

## 1. Code Map

This AGENTS.md applies to `static_core/compiler/aot/`. This directory owns the compiler-side model, product description, and runtime-consumable contract for AOT/LLVM AOT products; the broader workflow is routed by `static_core/compiler/AGENTS.md`.

Key areas:

| Path | Responsibility | Risk |
|---|---|---|
| `aot/` | AOT product model and runtime consumption information | Contract changes affect runtime loading and diagnostics |
| `aot_builder/` | AOT/LLVM AOT product assembly | Proves product generation only, not runtime use |
| `../tools/paoc/` | `ark_aot` user entry point | Input context, profile, mode, and location semantics must stay consistent |
| `../tools/aotdump/` | AOT diagnostics | Product contract changes must remain inspectable |

## 2. Knowledge Routing

| Scenario / Keywords | Read first |
|---|---|
| AOT / `.an` / paoc / runtime load / fallback / aotdump | `../docs/knowledge/aot-workflow.md` |
| LLVM AOT / LLVM backend / fallback / `paoc-mode=llvm` | `../docs/knowledge/llvm-aot-workflow.md` |
| AOT PGO / `.ap` / profile context | `../../runtime/jit/AGENTS.md`, `../docs/aot_pgo.md` |
| shared compiler workflow impact | `../docs/knowledge/compiler-workflows.md` |
| runtime linking / class linker / compiled runtime boundary | `../docs/knowledge/compiled-runtime-boundary.md` |

In the plan, state the matched scenario, documents read, and AOT/LLVM AOT constraints found.

## 3. Pre-coding Safety Checks

- Must prove pointer safety and GC visibility at AOT/runtime boundaries; avoid UAF, leaks, and null dereference, or cite ownership/lifetime and `SaveState`/roots/stack-map evidence.
- Must treat `CodeInfo`, stack maps, inline info, roots masks, bridge frames, and deopt metadata as correctness data.
- Must prove class context, panda file set, locations, checksums, and profile context match or reject clearly for AOT/AOT-PGO changes.
- Must prove LLVM backend participation separately from ARK AOT fallback for LLVM AOT claims.

## 4. Boundaries

- Do not treat product generation success as AOT/LLVM AOT runtime success.
- Do not change the AOT product contract without updating diagnostics, runtime-loading impact, and fallback explanation.
- Do not use loose matching to hide inconsistent input context, profile, or application file sets.
- Do not ignore shared compiler pipeline impact on JIT/OSR in AOT-only fixes.
- Do not loosen AOT/profile matching or count LLVM fallback as LLVM coverage.

Ask before changing product compatibility, profile matching, fallback policy, runtime-loading semantics, metadata layout/requirements, fallback diagnostics, aotdump visibility, or allowing LLVM fallback as LLVM coverage.

## 5. Verification Loop

| Change type | Minimum verification |
|---|---|
| AOT product | `ark_aot` generation plus dump/log evidence plus runtime load/use |
| LLVM AOT product | LLVM path evidence plus fallback/failure-path explanation |
| AOT PGO | Successful profile match, mismatch rejection, and optimization-use evidence |
| Shared compiler change | Compiler gtest/checked coverage plus affected workflow explanation |

Done evidence must distinguish generation, inspection, runtime loading, and runtime use.
