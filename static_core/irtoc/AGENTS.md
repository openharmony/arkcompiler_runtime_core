# irtoc Agent Guide

## 1. Code Map

This AGENTS.md applies to `static_core/irtoc/`. This directory generates interpreter handlers, runtime/compiler fastpaths, nativeplus boundary code, and selected low-overhead intrinsic entry points.

Key areas:

| Path | Responsibility | Risk |
|---|---|---|
| `scripts/` | `.irt` source describing interpreter handlers and helpers | Complex semantic logic should not be moved into irtoc casually |
| `backend/` | Backend from irtoc to compiler IR/codegen | ABI, register, and boundary constraints affect generated code |
| `lang/` | DSL and validation | Looser validation can hide code-size, spill, and shape risks |
| `../plugins/ets/irtoc_scripts/` | ETS-specific irtoc extensions | ETS behavior must first respect language and runtime semantics |

## 2. Knowledge Routing

| Scenario / Keywords | Read first |
|---|---|
| `.irt` / generated interpreter / generated helper / validation / DSL | `docs/knowledge/irtoc-generated-code.md` |
| fastpath / nativeplus / FastPathPlus / CallFastPath / boundary entry | `docs/knowledge/irtoc-fastpath.md` |
| FastPathPlus / NativePlus / interpreter intrinsic ABI | `docs/knowledge/irtoc-fastpath.md`, `../docs/irtoc-intrinsics-for-interpreter.md` |
| compiler intrinsic / CallFastPath / runtime entrypoint | `../compiler/docs/knowledge/jit-workflow.md`, `../compiler/docs/knowledge/compiled-runtime-boundary.md` |
| OSR trigger / generated interpreter interaction | `../compiler/docs/knowledge/osr-workflow.md`, `../runtime/osr.*`, `../runtime/arch/*/osr_*` |
| ETS intrinsic / ETS fastpath | `../plugins/ets/compiler/AGENTS.md` |
| tests / checked / interpreter validation | `../tests/AGENTS.md` |

In the plan, state the matched scenario, documents read, and generated-code or ABI constraints found.

## 3. Pre-coding Safety Checks

- Must prove pointer safety and GC visibility for fastpath/nativeplus/generated-code boundaries; avoid UAF, leaks, and null dereference, or cite ownership/lifetime and `SaveState`/roots/stack-map evidence.
- Must keep object/reference values visible across runtime calls, safepoints, bridges, OSR triggers, and generated fastpaths.
- Must identify whether boundary ownership is `CallRuntime`, `CallFastPath`, `FastPathPlus`, `NativePlus`, managed lowering, or runtime metadata.
- Must preserve irtoc validation, matching/rejection diagnostics, generated ABI, stack walking, exception, GC, class-init, deopt, and OSR checks.

## 4. Boundaries

- Do not treat irtoc as the default optimization mechanism; first check whether managed code, C++ runtime, compiler pass, or codegen is more appropriate.
- irtoc is suitable only for short, hot, clearly bounded low-overhead paths; do not move long logic, complex branches, or broad business semantics into it.
- Do not hand-edit generated outputs; change the `.irt` or generator source and regenerate.
- Do not loosen validation only to make code generation pass unless ABI, code size, and target-architecture impact are also explained.
- Fastpath/nativeplus changes must consider runtime entrypoints, compiler codegen, and GC/exception/stack visibility.
- Do not bypass stack walking, exception, GC, class-init, deopt, OSR, or fastpath ABI checks.

Ask before loosening validation, changing ABI-visible generated code, weakening matching/rejection/fallback diagnostics, moving broad runtime/language behavior into irtoc, adding new `FastPath`/`FastPathPlus`/`NativePlus` entrypoints, or replacing existing managed/C++ logic with irtoc.

## 5. Verification Loop

| Change type | Minimum verification |
|---|---|
| DSL/source change | `irtoc_unit_tests` |
| backend/ABI change | `irtoc_compiler_unit_tests` plus runtime/compiler boundary validation |
| interpreter handler | `irtoc-interpreter-tests` |
| compiler fastpath | compiler checked/gtest + runtime execution |
| ETS fastpath | ETS compiler/checked/runtime execution layer |

Done evidence must name generated outputs inspected, tests run, and runtime/compiler boundary risk.
