# ETS Compiler Plugin

ETS language compiler plugin that extends the core compiler via generated and handwritten `.inl` / `.inl.h` hooks. It
adds ETS-specific bytecode lowering, intrinsic recognition, JS interop, native call optimization, and fastpath-aware
codegen.

## Directory Structure

- `optimizer/ir_builder/ets_inst_builder.{cpp,h}` - ETS bytecode to IR translation (LdObjByName, StObjByName, CallByName, Equals, Typeof, Nullcheck, etc.), templates generated from `ets_inst_templates.yaml`
- `optimizer/ir_builder/js_interop/` - JS interop IR building (conditional: `PANDA_ETS_INTEROP_JS`)
- `optimizer/optimizations/interop_js/` - JS interop optimization pass `InteropIntrinsicOptimization` (scope merging, wrap/unwrap elimination, partial redundancy elimination)
- `optimizer/ets_intrinsics_peephole.cpp` - ETS intrinsic peephole optimizations (StringEquals, Typeof, Equals, Nullcheck, etc.)
- `optimizer/ets_intrinsics_inlining_expansion.cpp` - Intrinsic inlining expansion (EscompatArrayGet/Set)
- `optimizer/ets_codegen_extensions.{cpp,h}` - Native call code generation (JIT uses direct pointers / AOT uses AOT data table lookup, deoptimizes on null)
- `ir_build_intrinsics_ets.cpp` - Intrinsic IR building (IsFinite, Signbit, TypedArray operations, Unsafe operations, etc.)
- `codegen_intrinsics_ets.cpp` - Intrinsic target code generation (Math, ArrayCopy, StringBuilder, etc.)
- `compiler_intrinsic_id_mapping_inl.h` - mapping between ETS intrinsics and compiler/runtime ids
- `runtime_adapter_ets.{cpp,h}` - `EtsBytecodeOptimizerRuntimeAdapter`, provides ETS runtime queries (String/StringBuilder identification, constructor detection, etc.)
- `ets_compiler_interface.h` - Compiler extension interface (virtual methods for EscompatArray, TypedArray, ArrayBuffer)
- `tools/paoc_compile_stdlib.sh` - helper script used by ETS tests to compile stdlib in AOT/JIT/LLVM/OSR modes
- `product_options.h` - Only `compiler-inline-external-methods` and `compiler-regex` are allowed in production builds

## Plugin Injection Points

Injected into the core compiler via `.inl.h` (declarations) and `.inl` (implementation/switch cases):

- IR building: `intrinsics_ir_build_ets.inl.h`, `intrinsics_ir_build_static_call_ets.inl`, `intrinsics_ir_build_virtual_call_ets.inl`
- Peephole: `intrinsics_peephole_ets.inl.h`
- Inlining expansion: `intrinsics_inlining_expansion_ets.inl.h`, `intrinsics_inlining_expansion_switch_case_ets.inl`
- Codegen: `codegen_intrinsics_ets_ext.inl.h`
- Pipeline: `ets_pipeline_includes.inl.h`, `ets_optimizations_after_unroll.inl`
- Loop optimizations: `loop_idioms_memmove_intrinsic_id_switch_case_ets.inl`, `loop_idioms_customize_memmove_intrinsic_switch_case_ets.inl`
- StringBuilder: `optimizer/optimizations/need_to_run_simplify_string_builder.inl`

## Runtime and FastPath Boundary

ETS compiler work often crosses plugin/runtime boundaries:

- many ETS intrinsics lower to `CallFastPath` or `CallRuntime` in `codegen_intrinsics_ets.cpp`
- native call optimization is implemented partly in ETS codegen extensions and partly in runtime entrypoints
- `runtime_adapter_ets.*` answers ETS-specific semantic questions used by optimizer passes
- runtime/compiler metadata for ETS intrinsics lives in `plugins/ets/runtime/ets_compiler_intrinsics.yaml`
- ETS runtime entrypoints and bridges live in `plugins/ets/runtime/ets_entrypoints.yaml`, `plugins/ets/runtime/ets_entrypoints.cpp`, and `plugins/ets/runtime/entrypoints/`
- fastpath and entrypoint work usually also touches `compiler/optimizer/code_generator/`, `runtime/compiler.cpp`, and `plugins/ets/runtime/`
- follow the repo-level `irtoc` policy in `../../../AGENTS.md` and the detailed irtoc guide in `../../../irtoc/AGENTS.md`
- ETS-specific rule of thumb: start from the ETS implementation first; if a stdlib helper is small and inlineable, prefer keeping it in ETS instead of adding a new `irtoc` fastpath
- when an ETS intrinsic crosses this boundary, expect to touch metadata, codegen, runtime entrypoints, and possibly `plugins/ets/irtoc_scripts/` together

When debugging a regression, decide first whether the issue starts in:

- ETS IR building - `optimizer/ir_builder/ets_inst_builder.*`, `ir_build_intrinsics_ets.cpp`
- ETS-only optimization - `optimizer/ets_intrinsics_peephole.cpp`, `optimizer/ets_intrinsics_inlining_expansion.cpp`, `optimizer/optimizations/interop_js/`
- ETS codegen / runtime-call lowering - `optimizer/ets_codegen_extensions.*`, `codegen_intrinsics_ets.cpp`
- runtime or entrypoint side - `plugins/ets/runtime/`, `runtime/compiler.cpp`, `irtoc/`

## Build

Automatically compiled into the `arkcompiler` target with ETS plugin enabled. CMake is the primary local workflow, but
GN integration also exists through the plugin-level build.

JS interop work requires both `-DPANDA_ETS_INTEROP_JS=ON` and `-DPANDA_JS_ETS_HYBRID_MODE=ON`.

```bash
ninja ets_tests_compiler_optimizer && ./bin-gtests/ets_tests_compiler_optimizer
ninja interop_compiler_unit_tests && ./bin-gtests/interop_compiler_unit_tests
ninja compiler_wrong_boot_context_test
ninja compiler_tests
ninja ets_interop_tests
```

`plugins/ets/tests/CMakeLists.txt` also defines stdlib compilation smoke targets that exercise compiler modes such as
AOT, JIT, LLVM AOT, and arm64 OSR through `tools/paoc_compile_stdlib.sh`.

Generated wiring for ETS compiler extensions comes from `plugins/ets/ets_plugin_options.yaml` and conditional
generation in `plugins/ets/compiler/CMakeLists.txt` such as `interop_intrinsic_kinds.h`.

## Validation

When changing ETS compiler logic, use more than one test layer:

- focused ETS compiler gtests in `plugins/ets/tests/compiler/optimizer/`
- boot/runtime context coverage in `plugins/ets/tests/compiler/wrong_boot_context_test/`
- ETS checked layers in `plugins/ets/tests/checked/` and `plugins/ets/tests/ets_es_checked/`
- runtime-facing execution layers such as `plugins/ets/tests/jitinterface/`, `ets_run_jit_ets_code_tests`,
  `ets_run_jit_osr_ets_code_tests`, and `ets_run_aot_ets_code_tests`
- JS interop coverage through `interop_compiler_unit_tests`, `ets_interop_tests`, and
  `ets_interop_js_checked_tests` when interop code changed

Use `docs/README.md` as the ETS compiler documentation and validation map when you need the full ETS-specific matrix.
Use `../../../tests/AGENTS.md` for runner workflows instead of duplicating URunner details here.

If you add or modify an intrinsic, usually you want:

1. a focused compiler gtest or interop gtest for IR/codegen shape,
2. a checked test for pass/disasm/event validation,
3. an execution test in ETS checked / interop / jitinterface if runtime interaction changed.
4. a baseline-vs-`irtoc` performance sanity check if the change was motivated by call-overhead savings; do not claim a
   win without comparing against the ETS or C++ baseline on the target configuration.

JS interop changes usually also need:

1. `interop_compiler_unit_tests`,
2. `ets_interop_tests` or a focused `*_gtests` subsuite from `plugins/ets/tests/interop_js/README.MD`,
3. both required build flags enabled: `PANDA_ETS_INTEROP_JS` and `PANDA_JS_ETS_HYBRID_MODE`.

For generic core-compiler validation, also use `../../../compiler/AGENTS.md`.

## Documentation

`docs/README.md` is the ETS compiler documentation index. Use it for the doc catalog, then open the focused note such as
`interop_intrinsic_opt_doc.md`, `native_call_opt_doc.md`, or `interop_string_constpool_doc.md`.
