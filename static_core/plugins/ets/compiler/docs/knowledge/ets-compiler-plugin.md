# ETS Compiler Plugin Knowledge

This file covers ETS-specific compiler plugin work: ETS bytecode lowering, intrinsics, native-call optimization, JS interop, runtime adapter queries, and ETS-aware codegen.

## Purpose

The ETS compiler plugin adapts the shared compiler to ETS language semantics. It should express ETS-specific knowledge without leaking unstable runtime assumptions or changing language behavior.

## Model

| Area | Purpose | Risk |
|---|---|---|
| ETS IR building | Translate ETS bytecode and intrinsics into compiler IR | Wrong semantic mapping affects all workflows |
| ETS optimizations | Apply ETS-specific simplifications and expansions | Shared mode impact must be checked |
| ETS codegen extensions | Lower native calls and runtime interactions | JIT/AOT/LLVM/OSR may need different evidence |
| Runtime adapter | Query ETS runtime facts for compiler decisions | Stale assumptions cause wrong optimization |
| JS interop | Optimize ETS/JS boundary when enabled | Build-flag and runtime-mode dependent |

## Subdomain Routing

| Task | Read / inspect |
|---|---|
| New ETS intrinsic or intrinsic bug | `static_core/plugins/ets/compiler/docs/README.md`, intrinsic build/codegen files, ETS runtime intrinsic metadata |
| Native call or ANI-sensitive codegen | `static_core/plugins/ets/compiler/docs/native_call_opt_doc.md`, ETS codegen extensions, ANI checked tests |
| JS interop optimization | `static_core/plugins/ets/compiler/docs/interop_intrinsic_opt_doc.md`, interop optimization sources, interop compiler tests |
| Interop string or constant-pool behavior | `static_core/plugins/ets/compiler/docs/interop_string_constpool_doc.md`, interop checked tests, frontend boundary notes |
| Workflow-specific regression | Core compiler workflow knowledge plus ETS execution coverage for the affected mode |

## Generated Hook Map

| Hook area | Route |
|---|---|
| IR building | `intrinsics_ir_build_ets.inl.h`, static/virtual call intrinsic switch files |
| Peephole | `intrinsics_peephole_ets.inl.h`, `optimizer/ets_intrinsics_peephole.cpp` |
| Inlining expansion | `intrinsics_inlining_expansion_ets.inl.h`, expansion switch files |
| Codegen | `codegen_intrinsics_ets.cpp`, `codegen_intrinsics_ets_ext.inl.h` |
| Pipeline insertion | `optimizer/ets_pipeline_includes.inl.h`, `optimizer/ets_optimizations_after_unroll.inl` |
| Runtime metadata | `static_core/plugins/ets/runtime/ets_compiler_intrinsics.yaml`, `ets_entrypoints.yaml` |

## Runtime And Fastpath Boundary

ETS compiler work often crosses plugin/runtime boundaries:

- many ETS intrinsics lower to `CallFastPath` or `CallRuntime` in `codegen_intrinsics_ets.cpp`;
- native call optimization is implemented partly in ETS codegen extensions and partly in runtime entrypoints;
- `runtime_adapter_ets.*` answers ETS-specific semantic questions used by optimizer passes;
- runtime/compiler metadata for ETS intrinsics lives in `plugins/ets/runtime/ets_compiler_intrinsics.yaml`;
- ETS runtime entrypoints and bridges live in `plugins/ets/runtime/ets_entrypoints.yaml`, `plugins/ets/runtime/ets_entrypoints.cpp`, and `plugins/ets/runtime/entrypoints/`;
- fastpath and entrypoint work usually also touches compiler codegen, runtime compiler support, and `plugins/ets/runtime/`.

When debugging a regression, decide first whether the issue starts in ETS IR building, ETS-only optimization, ETS codegen/runtime-call lowering, or the runtime/entrypoint side.

## Code and Test Anchors

- ETS IR/intrinsics start in `static_core/plugins/ets/compiler/`.
- ETS runtime facts start in `static_core/plugins/ets/runtime/`.
- Shared compiler pipeline impact starts in `static_core/compiler/optimizer/`.
- irtoc ETS extensions start in `static_core/plugins/ets/irtoc_scripts/`.
- Validation should include ETS compiler gtests, checked tests, jitinterface, interop tests, or target workflow runs as applicable.

## Build And Test Commands

ETS compiler code is compiled into the `arkcompiler` target when the ETS plugin is enabled. JS interop work requires both `-DPANDA_ETS_INTEROP_JS=ON` and `-DPANDA_JS_ETS_HYBRID_MODE=ON`.

```sh
ninja ets_tests_compiler_optimizer && ./bin-gtests/ets_tests_compiler_optimizer
ninja interop_compiler_unit_tests && ./bin-gtests/interop_compiler_unit_tests
ninja compiler_wrong_boot_context_test
ninja compiler_tests
ninja ets_interop_tests
```

Use more than one layer when runtime interaction or codegen changes:

- focused ETS compiler gtests in `plugins/ets/tests/compiler/optimizer/`;
- boot/runtime context coverage in `plugins/ets/tests/compiler/wrong_boot_context_test/`;
- ETS checked layers in `plugins/ets/tests/checked/` and `plugins/ets/tests/ets_es_checked/`;
- runtime-facing execution layers such as `plugins/ets/tests/jitinterface/`, `ets_run_jit_ets_code_tests`, `ets_run_jit_osr_ets_code_tests`, and `ets_run_aot_ets_code_tests`;
- JS interop coverage through `interop_compiler_unit_tests`, `ets_interop_tests`, and `ets_interop_js_checked_tests`.

## Boundaries

- Do not change ETS language behavior to fit a compiler optimization.
- Do not assume an ETS intrinsic is mode-neutral; check JIT, AOT, LLVM AOT, and OSR as applicable.
- ETS intrinsic boundary changes must identify owner: managed lowering, `CallRuntime`, `CallFastPath`, `FastPathPlus`, `NativePlus`, ANI/native-call optimization, or ETS runtime metadata.
- Do not add fastpaths before checking managed and runtime alternatives.
- Do not mix JS interop assumptions into normal ETS paths without build-flag gating.
- Do not bypass runtime boundary checks or weaken fastpath/NativePlus ABI requirements for ETS intrinsic coverage.
- Do not change frontend-owned interop encoding from this directory; route that work through the frontend boundary first.
- Do not treat ETS plugin tests as proof for core compiler behavior unless the shared workflow evidence is also present.

## Common Misroutes

- If an ETS intrinsic fails, check IR build mapping, peephole/expansion, and runtime entrypoint first.
- If AOT differs from JIT, check native-call lowering, AOT data, class context, and runtime boundary.
- If a failure is interop-only, check interop build flags and JS boundary assumptions.
- If method coverage is missing, check method eligibility, compiler workflow, and language semantics.
- If a native call crashes, check ANI mode, managed/native boundary state, entrypoint, and stack visibility.

## Modification Checklist

Before editing, confirm:

- Core compiler and runtime impact are stated even when the change appears ETS-only.
- JIT, AOT, LLVM AOT, and OSR coverage is listed for affected workflows.
- Non-compiled and compiled behavior match when visible behavior changes.
- Metadata, entrypoint, and fastpath owners are included when the change crosses runtime or irtoc.
- Pointer lifetime, non-null invariants, GC visibility, and compiled metadata impact are stated for ETS runtime-boundary changes.

## Verification

- IR/codegen shape is correct: ETS compiler gtest or checked test.
- Execution is correct: ETS execution or jitinterface test.
- Interop remains correct: interop gtest/checked when relevant.
- Workflow coverage is proven: target workflow logs, dumps, or tests.
