# ETS Compiler Documentation Map

Use `../AGENTS.md` for active ETS compiler work and this file as the documentation map for ETS-specific compiler notes.

## Start Here By Task

- new ETS intrinsic or intrinsic bug:
  - `../ir_build_intrinsics_ets.cpp`
  - `../codegen_intrinsics_ets.cpp`
  - `../compiler_intrinsic_id_mapping_inl.h`
  - `../../runtime/ets_compiler_intrinsics.yaml`
  - `../../runtime/ets_entrypoints.yaml`
- JS interop optimization:
  - `interop_intrinsic_opt_doc.md`
  - `../optimizer/optimizations/interop_js/`
  - `../../../tests/interop_js/tests/compiler/interop_intrinsics_opt_test.cpp`
  - build with `-DPANDA_ETS_INTEROP_JS=ON -DPANDA_JS_ETS_HYBRID_MODE=ON`
  - run `ninja interop_compiler_unit_tests` and `ninja ets_interop_tests`
- native call optimization / ANI-sensitive codegen:
  - `native_call_opt_doc.md`
  - `../optimizer/ets_codegen_extensions.*`
  - `../../runtime/ets_entrypoints.*`
  - `../../../tests/checked/ani/`
- string / const-pool interop work:
  - `interop_string_constpool_doc.md`
  - `../../../tests/interop_js/tests/checked/`

## Important Source Boundaries

- ETS IR builder: `../optimizer/ir_builder/ets_inst_builder.*`
- generated intrinsic hooks: `../intrinsics_ir_build_ets.inl.h`, `../intrinsics_ir_build_static_call_ets.inl`,
  `../intrinsics_ir_build_virtual_call_ets.inl`, `../intrinsics_peephole_ets.inl.h`,
  `../intrinsics_inlining_expansion_ets.inl.h`
- ETS-specific runtime queries: `../runtime_adapter_ets.*`
- runtime side of compiler entrypoints and intrinsics: `../../runtime/`
- shared compiler/runtime fastpaths: `../../../../irtoc/`, `../../../../runtime/compiler.cpp`

## Validation Matrix

Use more than one layer whenever runtime interaction or codegen changes:

- focused ETS compiler gtests:
  - `../../../tests/compiler/optimizer/`
  - target `ets_tests_compiler_optimizer`
- boot/runtime context checks:
  - `../../../tests/compiler/wrong_boot_context_test/`
  - target `compiler_wrong_boot_context_test`
- ETS checked tests:
  - `../../../tests/checked/`
  - `../../../tests/ets_es_checked/`
- JS interop end-to-end coverage:
  - target `ets_interop_js_gtests`
  - target `ets_interop_js_checked_tests`
- JIT/runtime interface coverage:
  - `../../../tests/jitinterface/`
- broad execution sweeps:
  - `ets_run_jit_ets_code_tests`
  - `ets_run_jit_osr_ets_code_tests`
  - `ets_run_aot_ets_code_tests`

Runner-driven ETS checked coverage is owned outside this directory. For URunner2 suites such as
`compiler_checker_suite` or `ets-es-checked`, use `../../../../tests/AGENTS.md` and the suite YAML files under
`../../../../tests/tests-u-runner-2/cfg/test-suites/`.

For core compiler dumps/logs/calling-convention/deopt guidance, also read `../../../../compiler/AGENTS.md` and
`../../../../compiler/docs/README.md`.

## Frontend Boundary

`interop_string_constpool_doc.md` references frontend code in the sibling `ets_frontend` checkout. When you need to
change that flow, the relevant tree is outside `static_core`; local URunner/interop docs refer to it through
`ARKCOMPILER_ETS_FRONTEND_PATH`.
