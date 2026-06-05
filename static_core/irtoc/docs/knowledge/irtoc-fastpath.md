# irtoc Fastpath Knowledge

This file covers irtoc FastPath, FastPathPlus, and NativePlus boundary code used by runtime/compiler workflows. Generated interpreter and general `.irt` source work is covered by `irtoc-generated-code.md`.

## Purpose

irtoc fastpaths provide tightly constrained generated code for very hot or boundary-sensitive paths. They should reduce boundary overhead without moving broad language semantics out of their natural owners.

## Model

| Area | Purpose | Risk |
|---|---|---|
| FastPath | Low-overhead compiler/runtime entry | ABI and GC/exception constraints are strict |
| NativePlus | Native ABI-facing generated path | Bridge assumptions must stay explicit |
| FastPathPlus | Emits paired FastPath and NativePlus forms | Both boundary variants must stay consistent |
| Validation | Guards size/spill/shape limits | Weakening checks hides ABI/code-size risk |

## ABI Variants

| Variant | Used by | Guardrail |
|---|---|---|
| FastPath | compiled code and compiler-generated calls | Follow compiler ABI, live-register, and stack-map expectations |
| NativePlus | native/interpreter-facing boundary calls | Keep bridge assumptions explicit; first argument and ABI shape matter |
| FastPathPlus | paired FastPath and NativePlus generation | Validate both variants; success in one does not prove the other |

arm32 still has irtoc codegen branches and register maps, but runtime interpreter selection stays on the C++ path there. Do not collapse those two facts into "arm32 unsupported."

## Choosing irtoc

Use irtoc primarily for short critical paths where reducing `FastPath` or boundary-call overhead matters more than the optimizer flexibility lost by the special irtoc compilation mode.

Before adding a new fastpath, compare it against inline managed code, a regular managed call, a C++ runtime or intrinsic implementation, and compiler/codegen support. The best fits are very hot trivial constructors/allocation helpers or tiny low-level instruction sequences that managed source or C++ cannot lower into the desired machine instructions.

Keep irtoc bodies small. If a larger irtoc implementation materially outperforms the high-level or C++ version, investigate algorithmic issues or missing optimizer/codegen support before making irtoc the long-term solution.

## Code and Test Anchors

Owners are fastpath sources (`static_core/irtoc/scripts/`, `static_core/plugins/ets/irtoc_scripts/`), backend/ABI code (`static_core/irtoc/backend/`), DSL validation (`static_core/irtoc/lang/`), and runtime/compiler boundary checks (`static_core/runtime/`, `static_core/compiler/optimizer/`). Validate with irtoc unit/backend tests plus focused runtime or checked execution when the boundary is involved.

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| irtoc pipeline, modes, generated outputs | `static_core/docs/irtoc.md` |
| interpreter-facing intrinsic generation | `static_core/docs/irtoc-intrinsics-for-interpreter.md` |
| compiler/runtime boundary map | `static_core/compiler/docs/knowledge/compiled-runtime-boundary.md` |

## Test Commands

```sh
ninja irtoc_unit_tests
ninja irtoc_compiler_unit_tests
ninja irtoc-interpreter-tests
```

## Debugging Hints

- For fastpath bugs, correlate `codegen_fastpath.*` with `compiler/optimizer/code_generator/codegen-inl.h`, `runtime/compiler.cpp`, and runtime entrypoints.
- For nativeplus/boundary issues, inspect `codegen_nativeplus.*` and runtime bridge/entrypoint code.
- For compiler-generated fastpath issues, also inspect `tests/checked/osr_irtoc_test.pa` and `tests/checked/compiler_to_interpreter.pa`.

## Boundaries

- Do not choose an irtoc fastpath before comparing managed code, C++ runtime, and compiler/codegen fixes.
- Do not put broad language semantics into irtoc.
- Do not weaken validation just to fit a larger helper.
- Do not carry raw object pointers across GC, safepoints, runtime calls, deopt, OSR, bridges, or generated fastpaths unless root/`SaveState` visibility is proven.
- Do not weaken fastpath ABI, validation, matching, rejection, or fallback diagnostics.
- Do not change fastpath/nativeplus behavior without runtime and compiler boundary validation.

## Common Misroutes

- If a fastpath is requested for speed, first check whether managed code, C++, or compiler/codegen work can solve it.
- If validation fails, check helper size and complexity before weakening validator strictness.
- If a fastpath crashes, check ABI, live registers, entrypoints, stack visibility, and GC visibility.
- If NativePlus differs from FastPath, check ABI variant assumptions and bridge ownership.

## Modification Checklist

Before editing, confirm:

- Alternative managed, C++, and compiler/codegen implementations were considered.
- The path is small, hot, and either measured or clearly justified.
- Runtime/compiler boundary validation is planned when the path crosses that boundary.
- Pointer lifetime, non-null invariants, and object/reference visibility are proven for values crossing the boundary.
- ETS compiler ownership is respected when ETS behavior is involved.

## Verification

- Fastpath source is valid: irtoc unit tests.
- Backend generation is valid: irtoc compiler/backend tests.
- Runtime behavior is correct: execution tests crossing the target boundary.
- Compiler interaction is safe: checked tests, gtests, or runtime execution as applicable.
