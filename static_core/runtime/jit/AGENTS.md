# Runtime JIT Profile Agent Guide

## 1. Code Map

This AGENTS.md applies to `static_core/runtime/jit/`. This directory owns runtime profile data structures, `.ap` profile save/load, and the profile representation consumed by AOT PGO; it does not own JIT queues, workers, OSR entry, or compiled-code installation.

Key areas:

| Path | Responsibility | Risk |
|---|---|---|
| `profiling_data.*` | Runtime profile data | Data semantics affect JIT/AOT optimization decisions |
| `profiling_loader.*`, `profiling_saver.*` | Profile load/save | Save conditions or load-context mismatch can make profiles unusable |
| `libprofile/` | `.ap` on-disk profile representation | Format and class context are compatibility boundaries |
| `profile_saver_worker.*` | Incremental/background saving | Not the same as a JIT compiler worker |

## 2. Knowledge Routing

| Scenario / Keywords | Read first |
|---|---|
| `.ap` / profile save-load / inline cache / branch profile / throw profile / class context | `docs/knowledge/profile-data.md` |
| AOT PGO / `--paoc-use-profile` / profile mismatch | `../../compiler/docs/knowledge/aot-workflow.md`, `../../compiler/docs/aot_pgo.md`, `docs/knowledge/profile-data.md` |
| JIT hotness / JIT runtime orchestration / code install | `../../compiler/docs/knowledge/jit-workflow.md`, `../compiler*.h`, `../compiler_*.h` |
| OSR trigger / OSR execution | `../../compiler/docs/knowledge/osr-workflow.md`, `../osr.*`, `../arch/*/osr_*` |
| profile diagnostics / aptool | `../../compiler/AGENTS.md`, `../../docs/aptool.md` |

In the plan, state the matched scenario, documents read, and profile-context constraints found.

## 3. Pre-coding Safety Checks

- Must prove pointer safety for profile/class-context data; avoid UAF, leaks, and null dereference, or cite ownership/lifetime/invariant evidence when no null check exists.
- Must prove AOT-PGO class context, panda file set, locations, checksums, and profile context match or reject clearly.
- Must keep profile diagnostics and aptool visibility strong enough to explain mismatch, rejection, and consumption.
- Must not use profile save/load evidence as proof of JIT selection, compilation, entrypoint install, or compiled execution.

## 4. Boundaries

- Do not start JIT worker, compiler queue, OSR entry, or code-installation work from this directory.
- Profile data is valid for optimization only when class context, panda file set, and runtime context match.
- Do not use `force` or loose matching to hide a mismatch between the profile and the current application context.
- Keep diagnosable failure information when changing `.ap` representation, save conditions, or load conditions.
- Profile-guided optimization claims must show whether the profile was actually loaded and consumed.
- Do not loosen AOT/profile matching or weaken rejection diagnostics to increase profile reuse.

Ask before changing `.ap` compatibility, context matching, save/load rejection behavior, profile option semantics, aptool visibility, or AOT-PGO matching requirements.

## 5. Verification Loop

| Change type | Minimum verification |
|---|---|
| Profile save | Generate `.ap` and confirm target data is readable by tooling |
| Profile load | Matching context loads successfully; mismatching context is clearly rejected |
| AOT PGO | `ark_aot` has compiler/diagnostic evidence after using the profile |
| Profile format or compatibility | New, old, and invalid input paths have tests or explicit explanation |

Done evidence must include profile save/load status, tooling output when relevant, and mismatch behavior if compatibility is touched.
