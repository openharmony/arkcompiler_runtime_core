# JIT Workflow Knowledge

This file covers the JIT workflow for ArkTS-Sta compiler/runtime work. It is self-contained for JIT routing and boundaries; open neighboring AGENTS only when the task crosses into their owned code.

## Purpose

JIT compiles hot methods while the program is running. Its goal is steady-state performance without breaking runtime responsiveness, profiling meaning, stack visibility, GC safety, or deoptimization.

## Trigger Model

| Trigger / state | Meaning | Risk |
|---|---|---|
| `--compiler-enable-jit` | Allows runtime JIT compilation | Disabled JIT can look like a compiler failure |
| Hotness threshold | Controls when a method becomes a candidate | Too-low thresholds change execution shape and timing |
| `force_jit` / hotness zero | Forces early compilation in tests | Useful for coverage, but not representative of normal warmup |
| Async vs sync JIT | Determines whether compilation is backgrounded | Async timing can hide races or make evidence harder to read |
| Compiled entry install | Makes compiled code executable by runtime | Compilation without installation is not JIT success |

## State Lifecycle

| State | Owner | What to prove |
|---|---|---|
| Candidate selection | Runtime hotness/policy | Method is eligible and selected |
| Graph compilation | Compiler pipeline | The intended graph and pass/codegen path ran; shared fixes consider AOT/OSR |
| Code installation | Runtime/compiler boundary | Runtime can call the compiled entry |
| Compiled execution | Runtime | The compiled entry was actually used |
| Deopt/fallback | Runtime boundary | Returning to interpreter preserves state and diagnostics |

## Code and Test Anchors

- Shared compiler behavior starts in `static_core/compiler/optimizer/`.
- Runtime scheduling and installation start in `static_core/runtime/compiler*` and `static_core/runtime/compiler_*`.
- Profile data used by optimization starts in `static_core/runtime/jit/`.
- JIT workflow tests include `static_core/tests/tests-u-runner-2/cfg/workflows/*jit*.yaml` and `static_core/tests/checked/`.

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| JIT flags, force-jit caveats, dumps | `static_core/compiler/docs/compiler_doc.md` |
| In-place/background compilation entry | `static_core/compiler/docs/compilation_start.md` |
| JIT/profile data model | `static_core/runtime/jit/docs/knowledge/profile-data.md`, `static_core/docs/bytecode_profiling.md` |
| bridge / code install / stack map / deopt / compiled-runtime boundary | `static_core/compiler/docs/knowledge/compiled-runtime-boundary.md` |

## Boundaries

- Do not treat `runtime/jit/` as the whole JIT subsystem; it owns profile data, not queues or code installation.
- Do not claim JIT success from compilation alone; the method must be installed and executed as compiled code.
- JIT claims must prove selection, compilation, entrypoint install, and actual compiled execution.
- Do not treat compiler-only offline validation such as `ark_aot --paoc-mode=jit` as proof of runtime JIT selection, code installation, or compiled execution.
- Do not make JIT-only assumptions in shared passes unless AOT/OSR impact is checked.
- Stack maps, `CodeInfo`, GC visibility, bridge metadata, and deopt metadata are required runtime contract, not optional debug detail.
- Treat inline info, roots masks, bridge frames, and deopt metadata as correctness data.
- Do not remove deopt, stack, or GC diagnostics to stabilize a JIT test.

## Ask Before

- changing JIT trigger policy, hotness defaults, `force_jit` expectations, async-vs-sync defaults, install/status ordering, or entrypoint publication;
- changing when fallback is allowed, how mode selection is decided, or what counts as successful compiled execution;
- changing stack-map, `CodeInfo`, bridge, roots, GC, inline info, or deopt metadata ownership or requirements.

## Common Misroutes

- If a method stays interpreted, check hotness threshold, JIT enable flags, async scheduling, and filters first.
- If force-JIT behaves differently, check force-JIT runtime caveats and loaded AOT interaction.
- If JIT crashes after compile, check code installation, bridge/entrypoint, stack map, and deopt.
- If JIT performance regresses, check profile quality, pass decisions, runtime fallback, and benchmark mode.

## Modification Checklist

Before editing, confirm:

- Shared impact is listed when the change is not strictly JIT-specific.
- Runtime-side validation is included when runtime state affects the result.
- Profile source and match assumptions are explicit when profile data is involved.
- Logs, events, dumps, or tests prove compiled-code use when compiled execution is claimed.

## Verification

- Candidate is selected: hotness/profile/filter behavior is visible.
- Code is installed: runtime evidence shows compiled entry use.
- Behavior is equivalent: interpreted and JIT execution agree.
- Runtime boundary is safe: exception, stack, GC, or deopt path is covered when relevant.
- Object/reference values live across runtime calls, safepoints, deopt, or bridges stay visible through `SaveState`, roots, stack maps, or handles.
