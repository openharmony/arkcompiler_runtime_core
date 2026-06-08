# OSR Workflow Knowledge

This file covers On-Stack Replacement for ArkTS-Sta compiler/runtime work. It focuses on loop-entry compilation and interpreter-to-compiled transition boundaries.

## Purpose

OSR lets a hot running loop transfer from interpreted execution into compiled code without restarting the method. Its value is reducing long-running interpreted loop cost while preserving live program state.

## Trigger Model

| Trigger / state | Meaning | Risk |
|---|---|---|
| Backward branch hotness | Loop execution drives OSR candidacy | Non-loop hotness evidence does not prove OSR |
| `--compiler-enable-osr` | Allows OSR transitions | Disabled OSR can look like a compiler or test miss |
| Interpreter frame/live state | Source of values for loop-entry transition | Wrong mapping or frame assumptions corrupt state silently |
| Deoptimized frame flag | Blocks OSR from unsuitable frames | Ignoring it can re-enter compiled code unsafely |
| Architecture/test gating | Some OSR validation is arch-gated | A skipped OSR test is not evidence |

## OSR-Specific Differences

| Topic | Normal JIT/AOT method entry | OSR |
|---|---|---|
| Entry point | Method starts at bytecode entry | Execution resumes inside a hot loop |
| Inputs | Arguments and method frame state | Live interpreter values and loop state |
| Stack shape | Normal compiled frame setup | Interpreter-to-compiled transition frame |
| Failure signal | Method fallback or compile failure | Wrong live state, transition crash, or missed OSR event |

## Code and Test Anchors

- OSR compiler behavior starts in `static_core/compiler/optimizer/` and `static_core/compiler/tests/osr_test.cpp`.
- Runtime transition starts in `static_core/runtime/osr.*` and `static_core/runtime/arch/*/osr_*`.
- Generated interpreter interaction starts in `static_core/irtoc/`.
- OSR validation should include `static_core/tests/checked/osr*.pa` or `static_core/runtime/tests/osr_code_test.cpp`.

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| OSR design and runtime interaction | `static_core/docs/on-stack-replacement.md` |
| OSR compiler mode and flags | `static_core/compiler/docs/compiler_doc.md` |
| Interpreter trigger context | `static_core/docs/interpreter-knowledge.md`, `static_core/docs/design-of-interpreter.md` |
| live state / bridge / stack map / runtime transition | `static_core/compiler/docs/knowledge/compiled-runtime-boundary.md` |
| interpreter trigger / generated handler | `static_core/irtoc/docs/knowledge/irtoc-generated-code.md` |

## Boundaries

- Do not treat OSR as normal JIT with a different entry point; live state and entry semantics are different.
- Do not change loop optimizations without checking OSR-only entry constraints.
- Do not treat compiler-only offline validation such as `ark_aot --paoc-mode=osr` as proof that runtime OSR trigger, live-state capture, or loop-entry transition worked.
- Do not claim OSR coverage from a normal compiled run; OSR must actually trigger.
- OSR loop-entry and live-state invariants are strict; deoptimized frames must not be allowed to re-enter OSR.
- OSR changes must prove `SaveStateOsr`, OSR stack map, live-state reconstruction, and deoptimized-frame exclusion.
- Stack maps, `CodeInfo`, GC visibility, bridge metadata, and deopt metadata are required for OSR transitions.
- Do not bypass stack map or frame reconstruction checks to make an OSR case pass.

## Ask Before

- changing OSR trigger policy, backward-branch hotness defaults, or loop-entry eligibility rules;
- changing fallback, deopt, or mode-selection behavior that decides when frames may enter or re-enter OSR;
- changing `SaveStateOsr`, stack-map, `CodeInfo`, bridge, GC, deopt, or live-state metadata requirements.

## Common Misroutes

- If OSR never triggers, check interpreter mode, hotness threshold, loop shape, and arch/test gating first.
- If an OSR result differs, check live value mapping, phi/state handling, and deopt/exception paths.
- If a crash is OSR-only, check frame transition, stack walker, bridge, and GC root visibility.
- If an OSR checked test is skipped, check release, architecture, and sanitizer gating.

## Modification Checklist

Before editing, confirm:

- Normal JIT/AOT impact is stated when the change is not strictly OSR-specific.
- State mapping is tested when live values are affected.
- Runtime boundary validation is included when the transition crosses runtime frames.
- The test proves OSR entry, not only loop execution, when coverage is claimed.

## Verification

- OSR triggers: event/log/checked evidence shows OSR entry.
- State is correct: values before and after transition match.
- Runtime boundary is safe: exception, deopt, stack, or GC-sensitive cases are covered when relevant.
- Object/reference values live across OSR, deopt, runtime calls, or safepoints stay visible through `SaveState`, roots, stack maps, or handles.
- Shared compiler paths hold: related JIT/AOT tests are considered for shared changes.
