# bytecode_optimizer (static_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/static_core/bytecode_optimizer`**.

---

## What is bytecode_optimizer (static_core)?

**bytecode_optimizer (static_core)** is the Ark bytecode optimizer for the **static VM / ArkTS** toolchain. It takes a **pandasm::Program** and panda file, builds **compiler::Graph** per method, runs a level-based optimization pipeline (including canonicalization, peepholes, const array resolver, call devirtualization, LICM, LSE, CSE, branch elimination, etc.), then does register allocation and encoding and generates bytecode back into **pandasm::Function**. There is **no** separate analysis phase (no `AnalysisBytecode` or `BytecodeAnalysisResults`).

This is a **different** implementation from **`arkcompiler/runtime_core/bytecode_optimizer`** (the dynamic / runtime_core bytecode optimizer). The two use different namespaces (`ark::bytecodeopt` vs `panda::bytecodeopt`), different pass pipelines, and different build targets (`libarktsbytecodeopt` vs `libarkbytecodeopt`). The **runtime_core** version has `AnalysisBytecode`, `BytecodeAnalysisResults`, `ModuleConstantAnalyzer`, and constant propagation for module/branch elimination; the **static_core** version has **no** analysis phase and instead adds **CheckResolver**, **Canonicalization**, **BytecodeOptPeepholes**, **ConstArrayResolver**, **CallDevirtualization**, and plugin-based codegen/runtime adapter.

Main capabilities:

- **Optimization only**: **OptimizeBytecode(prog, maps, pandafileName, isDynamic, hasMemoryPool)** — iterates non-external methods: bytecode → IR (IrBuilder) → **RunOptimizations** → BytecodeGen → debug info propagation. **RunOptimizations(graph, iface)** is the core pass sequence and is also part of the public API.
- **Opt levels**: Level 0 = no optimizations. Level 1 = ValNum, Lowering, MoveConstants (dynamic) or Peepholes, Canonicalization, Lowering, MoveConstants, BytecodeOptPeepholes (static). Level 2 = longer pipeline including ConstArrayResolver, BranchElimination, SimplifyStringBuilder, ValNum, IfMerging, Cse, Peepholes, CallDevirtualization, Licm, Lse, Cse, BranchElimination, Canonicalization, Lowering, MoveConstants, BytecodeOptPeepholes (and **CheckResolver** at the start for all levels). Then Cleanup, RegAccAlloc, RegAlloc, RegEncoder.
- **Static-specific passes**: **CheckResolver**, **Canonicalization**, **BytecodeOptPeepholes**, **ConstArrayResolver**, **CallDevirtualization**. No constant propagation or module constant analyzer.
- **Plugin-based codegen and runtime**: Codegen visitors, intrinsics, reg_encoder visitors, and runtime adapter are extended via **plugin_bytecodeopt** sources and generated inc/cpp from plugin YAMLs (**runtime_adapter_inc**, **codegen_visitors_inc**, **codegen_intrinsics_cpp**, **codegen_call_intrinsics_inc**, **get_intrinsic_id_inc**, **reg_encoder_visitors_inc**).

Public entry points are in **optimize_bytecode.h**: **RunOptimizations**, **OptimizeBytecode** (namespace **ark::bytecodeopt**). Built as **libarktsbytecodeopt** (shared) and **libarktsbytecodeopt_frontend_static** (static).

---

## High-level pipeline and data flow

1. **Input**: **OptimizeBytecode(prog, maps, pandafileName, isDynamic, hasMemoryPool)** — open panda file, iterate classes and non-external methods.

2. **Per method**:
   - Build **BytecodeOptIrInterface(maps, prog)**; open method's code, create **compiler::Graph** with **BytecodeOptimizerRuntimeAdapter** (or plugin runtime), run **IrBuilder**.
   - **RunOptimizations(graph, &irInterface)**:
     - **CheckResolver**; **Cleanup**.
     - If **dynamic**: ValNum, Lowering, MoveConstants. If **opt level 1**: Peepholes, Canonicalization, Lowering, MoveConstants, BytecodeOptPeepholes. If **opt level 2**: ConstArrayResolver, BranchElimination, SimplifyStringBuilder, ValNum, IfMerging, Cse, Peepholes, CallDevirtualization, Licm, Lse, ValNum, Cse, BranchElimination, Canonicalization, Lowering, MoveConstants, BytecodeOptPeepholes (ConstArrayResolver and SimplifyStringBuilder use **iface** where needed).
     - Cleanup, **RegAccAlloc**, Cleanup, **RegAlloc**, **RegEncoder**.
   - **BytecodeGen** (with plugin codegen) writes back to **pandasm::Function**; line/column propagation, update regs_num.

3. **Output**: Modified **pandasm::Program** (optimized function bodies), ready to be emitted to `.abc`.

Key data structures:

- **compiler::Graph** per method (built with **BytecodeOptimizerRuntimeAdapter** or plugin runtime).
- **BytecodeOptIrInterface**: maps, program, **GetPcInsMap**, GetMethodIdByOffset, GetStringIdByOffset, etc.; **IsMethodStatic** (static_core). No BytecodeAnalysisResults.
- **Options** (**g_options**): from **options.yaml** (opt-level, canonicalization, const-resolver, method-regex, skip-methods-with-eh, bytecode-opt-peepholes, bytecode-opt-call-devirtualize, const-array-resolver). Generated **bytecodeopt_options_gen.h**.

---

## Repository layout (where things live)

### Entry and orchestration

- **optimize_bytecode.h** / **optimize_bytecode.cpp** — **RunOptimizations(graph, iface)**, **OptimizeBytecode(prog, maps, pandafileName, isDynamic, hasMemoryPool)**; per-method skip logic (method-regex, skip-methods-with-eh, frame size); compiler options; BuildMapFromPcToIns, line/column propagation. Namespace **ark::bytecodeopt**.

### Passes and utilities (static_core only)

- **check_resolver.h** / **check_resolver.cpp** — **CheckResolver** pass (runs first in RunOptimizations).
- **canonicalization.h** / **canonicalization.cpp** — **Canonicalization** pass.
- **bytecodeopt_peepholes.h** / **bytecodeopt_peepholes.cpp** — **BytecodeOptPeepholes** pass.
- **const_array_resolver.h** / **const_array_resolver.cpp** — **ConstArrayResolver** pass (opt level 2; uses iface).
- **call_devirtualization.h** / **call_devirtualization.cpp** — **CallDevirtualization** pass.
- **common.h** / **common.cpp** — Common constants and helpers (e.g. AccReadIndex, GetStringFromPandaFile).
- **bytecode_encoder.h** — Encoder helpers (immediate encoding, etc.).

### Shared with runtime_core (sources under static_core or referenced from ark_root)

- **reg_acc_alloc.h** / **reg_acc_alloc.cpp** — **RegAccAlloc** (before RegAlloc).
- **reg_encoder.h** / **reg_encoder.cpp** — **RegEncoder** (after RegAlloc; register renumbering, spills, check_width).
- **codegen.h** / **codegen.cpp** — **BytecodeGen** (IR → pandasm instructions); plugin codegen via generated visitors/intrinsics.
- **ir_interface.h** — **BytecodeOptIrInterface** (maps, prog, GetMethodIdByOffset, GetStringIdByOffset, GetPcInsMap, IsMethodStatic, etc.).
- **runtime_adapter.h** / **runtime_adapter.cpp** — **BytecodeOptimizerRuntimeAdapter** (implements compiler RuntimeInterface); **generated/runtime_adapter_inc.h** injects plugin-specific runtime.

### Options and generated code

- **options.yaml** — Opt-level, canonicalization, const-resolver, method-regex, skip-methods-with-eh, bytecode-opt-peepholes, bytecode-opt-call-devirtualize, const-array-resolver. Namespace **ark::bytecodeopt**.
- **bytecodeopt_options.h** — Includes **generated/bytecodeopt_options_gen.h**; **g_options** in optimize_bytecode.cpp.
- **templates/** — ISA and plugin-driven codegen: **insn_selection.*.erb**, **check_width.*.erb**, **codegen_visitors.inc.erb**, **codegen_intrinsics.cpp.erb**, **codegen_call_intrinsics.inc.erb**, **reg_encoder_visitors.inc.erb**, **runtime_adapter_inc.h.erb**, **get_intrinsic_id.inc.erb**. Generated into **$target_gen_dir/generated/**.

### Plugin integration

- **plugin_libarkbytecodeopt_sources** — Extra sources from plugins (e.g. ETS bytecode optimizer).
- **plugin_bytecodeopt_deps** — Plugin deps. **plugin_bytecodeopt_configs** — Plugin configs.
- Generated **runtime_adapter_inc.h**, **codegen_visitors.inc**, **codegen_intrinsics.cpp**, **codegen_call_intrinsics.inc**, **get_intrinsic_id.inc**, **reg_encoder_visitors.inc** depend on **ark_plugin_options_yaml** (and for call intrinsics, runtime intrinsics.yaml).

### Tests

- **tests/** — **reg_acc_alloc_test**, **reg_encoder_test**, **codegen_test**, **canonicalization_test**, **check_resolver_test**, **const_array_resolver_test**, **bytecodeopt_peepholes_test**, **bytecodeopt_peepholes_runtime_test**, **bc_lowering_test**, **irbuilder_test**, **runtime_adapter_test**, **bitops_bitwise_and_test**, **benchmark/** (compare.py, run_benchmark.py), etc.

---

## Build and test

### Build

- **Libraries**: **libarktsbytecodeopt** (shared), **libarktsbytecodeopt_frontend_static** (static). Sources: **libarkbytecodeopt_sources** (bytecodeopt_peepholes, call_devirtualization, canonicalization, check_resolver, codegen, common, const_array_resolver, optimize_bytecode, reg_acc_alloc, reg_encoder, runtime_adapter) + **plugin_libarkbytecodeopt_sources**. Deps: **bytecodeopt_options_gen_h**, **codegen_call_intrinsics_inc**, **codegen_intrinsics_cpp**, **codegen_visitors_inc**, **get_intrinsic_id_inc**, **isa_gen_arkbytecodeopt_*** (check_width, insn_selection), **reg_encoder_visitors_inc**, **runtime_adapter_inc**, **libarktsassembler**, **libarktscompiler**, **libarktsbase**, **libarktsfile** (+ plugin_bytecodeopt_deps; optional libabckit deps when abckit_enable). Config: **bytecodeopt_public_config** (ENABLE_BYTECODE_OPT when enable_bytecode_optimizer && plugin_enable_bytecode_optimizer).
- **libarktsbytecodeopt_package** — Optional package that pulls in libarktsbytecodeopt when **enable_static_vm**.

From ark root (with static VM / bytecode optimizer enabled as needed):

```bash
./ark.py x64.release  # or the target that builds libarktsbytecodeopt
```

### Test

- Unit tests under **static_core/bytecode_optimizer/tests/**; run via the project test runner. Some tests depend on generated code or plugin-provided runtime/codegen.

---

## How to add a new feature (checklist)

### A) New optimization pass

1. Implement the pass (inherit **compiler::Optimization** or **compiler::Analysis**), implement **RunImpl()** and **GetPassName()**. If it needs **BytecodeOptIrInterface**, receive it via **RunOptimizations(..., iface)** and use it in the pass.
2. In **optimize_bytecode.cpp**, add the pass in the correct place in **RunOpts** (level 1 vs level 2, before/after RegAccAlloc). Use the **RunOpts<...>(graph, iface)** pattern and the **if constexpr** handling for passes that need **iface** (e.g. ConstArrayResolver, SimplifyStringBuilder).
3. Add tests under **tests/**.

### B) New option

1. Add the option in **options.yaml** (name, type, default, description). Regenerate **bytecodeopt_options_gen.h**.
2. In code, use **ark::bytecodeopt::g_options.Get...()** (or the generated accessor). Use it in **RunOptimizations** or **OptimizeBytecode** (e.g. skip logic, or to enable/disable a pass).

### C) Plugin codegen or runtime adapter

1. Plugin sources are wired via **plugin_libarkbytecodeopt_sources** and the plugin's subproject_sources. Generated **runtime_adapter_inc.h**, **codegen_visitors.inc**, **codegen_intrinsics.cpp**, **reg_encoder_visitors.inc**, etc., are produced from **templates/** and **ark_plugin_options_yaml**. Add or extend the plugin YAML and the corresponding `.erb` template so your plugin's runtime adapter or codegen visitors are included.
2. Ensure the plugin is in **plugin_bytecodeopt_deps** / **plugin_bytecodeopt_configs** and that **plugin_enable_bytecode_optimizer** is set so ENABLE_BYTECODE_OPT is defined when needed.

### D) New instruction or intrinsic in codegen

1. Codegen and reg_encoder instruction handling are largely **generated** from ISA and plugin YAMLs. Update the ISA (e.g. **bytecode_optimizer_isapi.rb**) and/or plugin templates (**codegen_visitors.inc.erb**, **codegen_intrinsics.cpp.erb**, **codegen_call_intrinsics.inc.erb**, **reg_encoder_visitors.inc.erb**) and regenerate. Do not hand-edit generated files under **generated/**.
2. Add or extend tests in **codegen_test**, **reg_encoder_test**, or peepholes tests as appropriate.

---

## Critical pitfalls / "do not do this"

- **Do not** mix code or APIs from **runtime_core/bytecode_optimizer** (panda::bytecodeopt, AnalysisBytecode, BytecodeAnalysisResults, ConstantPropagation, ModuleConstantAnalyzer) with **static_core/bytecode_optimizer** (ark::bytecodeopt, RunOptimizations/OptimizeBytecode only, CheckResolver, Canonicalization, BytecodeOptPeepholes, etc.). They are separate implementations with different pass orders and capabilities.
- **Do not** edit generated files (**generated/runtime_adapter_inc.h**, **codegen_visitors.inc**, **codegen_intrinsics.cpp**, **reg_encoder_visitors.inc**, **bytecodeopt_options_gen.h**, ISA-generated check_width/insn_selection) by hand. Change the `.erb` templates and/or options.yaml and regenerate.
- **Do not** assume **BytecodeAnalysisResults** or **AnalysisBytecode** exist in static_core; they do not. Any logic that needs module constant or analysis result must be implemented within the static_core pipeline (e.g. via a new pass or iface) or omitted.
- **Do not** skip **CheckResolver** or **RegEncoder**/ **RegAlloc** in the pipeline; the order and presence of passes are required for correctness. When adding a new pass, respect **Cleanup** and the existing ordering (e.g. Canonicalization before Lowering).
- When adding plugin-dependent code, ensure **plugin_bytecodeopt_deps** and **plugin_bytecodeopt_configs** are set and the plugin is enabled so the build and ENABLE_BYTECODE_OPT are consistent.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Entry and pipeline | **optimize_bytecode.h**, **optimize_bytecode.cpp** |
| Static-specific passes | **check_resolver.h**, **canonicalization.h**, **bytecodeopt_peepholes.h**, **const_array_resolver.h**, **call_devirtualization.h** |
| Shared passes and encoding | **reg_acc_alloc.h**, **reg_encoder.h**, **codegen.h**, **ir_interface.h**, **runtime_adapter.h** |
| Options | **options.yaml**, **bytecodeopt_options.h**, **generated/bytecodeopt_options_gen.h** |
| Templates and generated | **templates/** (runtime_adapter_inc, codegen_visitors, codegen_intrinsics, codegen_call_intrinsics, get_intrinsic_id, reg_encoder_visitors, insn_selection, check_width) |
| Build | **BUILD.gn** (libarktsbytecodeopt, libarktsbytecodeopt_frontend_static, isa_gen_arkbytecodeopt, bytecodeopt_options_gen_h, ark_gen_file targets for plugins) |
| Tests | **tests/** (reg_acc_alloc_test, reg_encoder_test, codegen_test, canonicalization_test, check_resolver_test, const_array_resolver_test, bytecodeopt_peepholes_*, etc.) |
