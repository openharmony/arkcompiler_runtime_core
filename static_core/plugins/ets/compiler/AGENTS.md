# ETS Compiler Plugin

ETS language compiler plugin that extends the core compiler via `.inl`/`.inl.h` injection files, providing ETS bytecode support, intrinsic optimizations, JS interop, and native call optimization.

## Directory Structure

- `optimizer/ir_builder/ets_inst_builder.{cpp,h}` - ETS bytecode to IR translation (LdObjByName, StObjByName, CallByName, Equals, Typeof, Nullcheck, etc.), templates generated from `ets_inst_templates.yaml`
- `optimizer/ir_builder/js_interop/` - JS interop IR building (conditional: `PANDA_ETS_INTEROP_JS`)
- `optimizer/optimizations/interop_js/` - JS interop optimization pass `InteropIntrinsicOptimization` (scope merging, wrap/unwrap elimination, partial redundancy elimination)
- `optimizer/ets_intrinsics_peephole.cpp` - ETS intrinsic peephole optimizations (StringEquals, Typeof, Equals, Nullcheck, etc.)
- `optimizer/ets_intrinsics_inlining_expansion.cpp` - Intrinsic inlining expansion (EscompatArrayGet/Set)
- `optimizer/ets_codegen_extensions.{cpp,h}` - Native call code generation (JIT uses direct pointers / AOT uses AOT data table lookup, deoptimizes on null)
- `ir_build_intrinsics_ets.cpp` - Intrinsic IR building (IsFinite, Signbit, TypedArray operations, Unsafe operations, etc.)
- `codegen_intrinsics_ets.cpp` - Intrinsic target code generation (Math, ArrayCopy, StringBuilder, etc.)
- `runtime_adapter_ets.{cpp,h}` - `EtsBytecodeOptimizerRuntimeAdapter`, provides ETS runtime queries (String/StringBuilder identification, constructor detection, etc.)
- `ets_compiler_interface.h` - Compiler extension interface (virtual methods for EscompatArray, TypedArray, ArrayBuffer)
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

## Build

Automatically compiled into the `arkcompiler` target with ETS plugin enabled (CMake only, no standalone BUILD.gn). JS interop requires `-DPANDA_ETS_INTEROP_JS=ON`.

```bash
ninja ets_tests_compiler_optimizer && ./bin-gtests/ets_tests_compiler_optimizer
```

## Documentation

Three design docs under `docs/`: interop intrinsic optimization, native call optimization, interop string constpool.
