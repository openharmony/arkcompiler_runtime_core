# ETS Compiler Plugin Agent Guide

## 1. Code Map

This AGENTS.md applies to `static_core/plugins/ets/compiler/`. This directory owns ETS language-plugin extensions to the core compiler, including ETS bytecode lowering, intrinsic recognition, ETS-specific optimization, JS interop, native calls, and fastpath-aware codegen.

Key areas:

| Path | Responsibility | Risk |
|---|---|---|
| `optimizer/ir_builder/` | ETS bytecode to IR | Frontend/bytecode semantic mistakes can affect all workflows |
| `optimizer/optimizations/`, `optimizer/*intrinsics*` | ETS-specific optimization and intrinsic handling | Shared JIT/AOT/LLVM/OSR impact must be evaluated explicitly |
| `optimizer/ets_codegen_extensions.*`, `codegen_intrinsics_ets.cpp` | ETS codegen/runtime-call boundary | Native call, entrypoint, and AOT data semantics must stay consistent |
| `runtime_adapter_ets.*`, `ets_compiler_interface.h` | ETS runtime semantic queries | Do not hard-code runtime state assumptions into the compiler |

## 2. Knowledge Routing

| Scenario / Keywords | Read first |
|---|---|
| ETS IR build / intrinsic / native call / JS interop / ETS codegen extension | `docs/knowledge/ets-compiler-plugin.md` |
| JIT / OSR / AOT / LLVM AOT workflow impact | `../../../compiler/docs/knowledge/compiler-workflows.md` |
| irtoc / fastpath / CallFastPath | `../../../irtoc/AGENTS.md`, `../../../irtoc/docs/knowledge/irtoc-fastpath.md` |
| runtime entrypoint / ETS runtime boundary | `../runtime/AGENTS.md`, `../../../compiler/docs/knowledge/compiled-runtime-boundary.md` |
| tests / checked / jitinterface / interop validation | `../../../tests/AGENTS.md` |

In the plan, state the matched scenario, documents read, and ETS/runtime/compiler constraints found.

## 3. Pre-coding Safety Checks

- Must prove pointer safety and GC visibility at ETS compiler/runtime/irtoc boundaries; avoid UAF, leaks, and null dereference, or cite ownership/lifetime and `SaveState`/roots/stack-map evidence.
- Must treat `CodeInfo`, stack maps, inline info, roots masks, bridge frames, and deopt metadata as correctness data when ETS lowering reaches compiled/runtime boundaries.
- Must identify ETS intrinsic boundary ownership: managed lowering, `CallRuntime`, `CallFastPath`, `FastPathPlus`, `NativePlus`, ANI/native-call optimization, or ETS runtime metadata.
- Must state JIT, OSR, AOT, and LLVM AOT impact for intrinsic, native-call, fastpath, or runtime metadata changes.

## 4. Boundaries

- Do not judge correctness only from the ETS plugin locally; ETS compiler changes often cross the core compiler, runtime, and irtoc.
- Do not change observable ETS language behavior for compiler coverage.
- Intrinsic, native call, and fastpath changes must consider JIT/AOT/LLVM AOT/OSR differences together.
- Do not re-home an ETS intrinsic between managed lowering, `CallRuntime`, `CallFastPath`, `FastPathPlus`, `NativePlus`, ANI/native-call optimization, or ETS runtime metadata without naming the owning runtime/irtoc boundary and the affected JIT/AOT/OSR coverage.
- JS interop changes must explicitly separate normal ETS compiler behavior from interop-enabled build behavior.
- Explain compatibility and verification scope before changing production-available options, runtime metadata, entrypoints, or generated hooks.
- Do not bypass runtime boundary checks or weaken fastpath/NativePlus ABI requirements for ETS intrinsic coverage.

Ask before:

- changing production-available options, runtime metadata, entrypoints, generated hooks, or JS interop boundary assumptions;
- re-homing an intrinsic between managed lowering, `CallRuntime`, `CallFastPath`, `FastPathPlus`, `NativePlus`, ANI/native-call optimization, or ETS runtime metadata;
- weakening null checks, runtime metadata requirements, boundary diagnostics, or JS interop boundary ownership.

## 5. Verification Loop

| Change type | Minimum verification |
|---|---|
| ETS IR/intrinsic shape | ETS compiler gtest or checked coverage |
| ETS codegen/runtime-call | ETS execution test plus runtime/compiler boundary validation |
| JS interop | interop compiler gtest + interop execution/checked |
| workflow smoke coverage | Smoke or focused case for affected JIT/AOT/LLVM/OSR modes |
| fastpath | Three-sided irtoc/runtime/compiler validation |

Done evidence must name ETS behavior impact, workflow coverage, validation run, and any runtime/irtoc boundary risk.
