# bytecode_optimizer (runtime_core) — Agent Guide

This document is the **full working guide** for **`arkcompiler/runtime_core/bytecode_optimizer`** (the **dynamic / runtime_core** bytecode optimizer).

There is a **separate** bytecode optimizer under **`arkcompiler/runtime_core/static_core/bytecode_optimizer`** (static / ArkTS). That one has a different pass pipeline (e.g. CheckResolver, Canonicalization, BytecodeOptPeepholes, ConstArrayResolver, CallDevirtualization), **no** `AnalysisBytecode`/`BytecodeAnalysisResults`, plugin-based codegen/runtime_adapter, and different build targets (`libarktsbytecodeopt`). Do not confuse the two.

---

## What is bytecode_optimizer (runtime_core)?

**bytecode_optimizer** is the Ark bytecode optimizer. It **analyzes and optimizes** **Panda/Ark bytecode** (the program representation behind `.abc` files) and produces a modified **pandasm::Program** that can be written back to `.abc`.

Main capabilities:

- **Analysis phase**: **AnalysisBytecode** — Performs module-level analysis on a Program (e.g. module constants, local/regular/namespace imports). Results are stored in **BytecodeAnalysisResults** for later optimization or external tools.
- **Optimization phase**: **OptimizeBytecode** — For each non-external method: bytecode → IR (**compiler::Graph**) → multiple optimization passes → register allocation and encoding → bytecode generation, writing back into **pandasm::Function**.

Public entry points are in **optimize_bytecode.h** (**AnalysisBytecode**, **RunOptimizations**, **OptimizeBytecode**). Namespace **panda::bytecodeopt**.

---

## High-level pipeline and data flow

Overall flow:

1. **Input**: **pandasm::Program** *, **PandaFileToPandaAsmMaps** *, panda file name (and flags such as **is_dynamic**, **has_memory_pool**).

2. **Analysis (optional)**  
   - **AnalysisBytecode**: Iterates over classes in the panda file, analyzes module info per record (local export / regular import / namespace import), runs **ModuleConstantAnalyzer** on **func_main_0**, and stores results in **BytecodeAnalysisResults** (indexed by record name).

3. **Optimization (per method)**  
   - Open panda file and iterate over each non-external method:  
     - Build **compiler::Graph** from bytecode using **BytecodeOptimizerRuntimeAdapter** and **IrBuilder**.  
     - **RunOptimizations(graph, &ir_interface)**: Cleanup → (optional) ConstantPropagation + BranchElimination → ValNum → Lowering → MoveConstants → Cleanup → RegAccAlloc → RegAlloc → Cleanup → RegEncoder.  
     - **BytecodeGen** encodes IR back into the instruction sequence of **pandasm::Function**.  
     - Debug info propagation (line/column numbers), update **regs_num**, etc.

4. **Output**: Modified **pandasm::Program** (with optimized function bodies), ready to be written to `.abc` by the assembler.

Key points:

- **Program-level representation**: **pandasm::Program** (e.g. **function_table**, **literalarray_table**).
- **Method-level IR**: **compiler::Graph** (shared with the compiler subsystem; same IR and pass infrastructure).
- **Analysis results**: Global static **BytecodeAnalysisResults** (per-record **BytecodeAnalysisResult** and bytecode maps), with thread safety provided by an internal mutex.

---

## Repository layout (where things live)

### Entry and orchestration

- **optimize_bytecode.h** / **optimize_bytecode.cpp**  
  - **AnalysisBytecode**, **OptimizeBytecode**, **OptimizePandaFile**, **OptimizeFunction**, **RunOptimizations**.  
  - Compiler options setup, per-method skip logic (regex, catch blocks, frame size), debug info propagation.

### Analysis and results

- **bytecode_analysis_results.h** / **bytecode_analysis_results.cpp**  
  - **BytecodeAnalysisResult**: constant export slots, local/regular/namespace import info for a single record.  
  - **BytecodeAnalysisResults**: global singleton-style access; create/get results and bytecode maps by record name.  
- **module_constant_analyzer.h** / **module_constant_analyzer.cpp**  
  - Walks function IR to analyze module constant initial values (e.g. stconstmodulevar in **func_main_0**); results go into **ModuleConstantAnalysisResult** and then into **BytecodeAnalysisResult**.

### IR and runtime abstraction

- **ir_interface.h**  
  - **BytecodeOptIrInterface**: resolves method/string/literalarray/type/field IDs from offsets, maintains PC→**pandasm::Ins** map, line/column numbers, literalarray storage; used by codegen and optimizations.  
- **runtime_adapter.h**  
  - **BytecodeOptimizerRuntimeAdapter**: implements **compiler::RuntimeInterface** over **panda_file::File** to provide method metadata and bytecode access for IR building and optimizations.

### Optimizations and encoding

- **constant_propagation/**  
  - SCCP-style constant propagation (module vars, compares, intrinsics, etc.), feeding branch elimination with constant conditions.  
  - **constant_value.h**, **lattice_element**, etc. for constant/lattice representation.  
- **reg_acc_alloc.h** / **reg_acc_alloc.cpp**  
  - Accumulator (acc) register allocation; runs before RegAlloc.  
- **reg_encoder.h** / **reg_encoder.cpp**  
  - After RegAlloc: reorder/renumber registers (locals, temps, args), handle instructions that can only encode [r0,r15], insert spills, and update **SetStackSlotsCount**.  
- **codegen.h** / **codegen.cpp** + **generated/**  
  - **BytecodeGen**: encodes **compiler::Graph** into **pandasm::Ins** sequence (visitor dispatch, try-catch, line/column numbers).  
  - Much of it is generated from templates/scripts (**codegen_visitors.inc**, **insn_selection**, **codegen_intrinsics.cpp**, etc.).

### Common and configuration

- **common.h** / **common.cpp**  
  - Register/argument count constants, **AccReadIndex**, **GetStringFromPandaFile**, etc.  
- **bytecodeopt_options.h** + **options.yaml**  
  - Options are generated from **options.yaml** into **generated/bytecodeopt_options_gen.h** (e.g. **opt-level**, **method-regex**, **skip-methods-with-eh**).  
- **bytecode_encoder.h**  
  - Subclasses **compiler::Encoder** for immediate encoding queries, etc.  
- **tagged_value.h**  
  - Helpers for tagged values.

### Templates and code generation

- **templates/**  
  - **codegen_visitors.inc.erb**, **insn_selection.\*.erb**, **codegen_intrinsics.cpp.erb**, **reg_encoder_visitors.inc.erb**, **check_width.\*.erb**, etc.; generated into **generated/** via **ark_isa_gen** / **ark_gen_file**.  
- **bytecode_optimizer_isapi.rb**  
  - ISA/plugin description; used to generate codegen/reg_encoder visitors and instruction selection.

### Tests

- **tests/**  
  - Unit tests: **analysis_bytecode_test**, **constant_propagation_test**, **codegen_test**, **reg_encoder_test**, **optimize_bytecode_test**, etc.  
  - Some tests rely on `.abc` files produced from JS (**tests/js/*.js** compiled with es2abc).  
  - **tests/BUILD.gn** uses **es2abc_gen_abc** to build test abc files and links against **libarkbytecodeopt**.

---

## Build and test

### Build

- **Library targets**: **libarkbytecodeopt** (shared) or **libarkbytecodeopt_frontend_static** (static).  
- **Dependencies**: **libarkcompiler**, **libarkassembler**, **libpandafile**, **libpandabase**, and generated headers/sources (bytecodeopt_options, codegen_visitors, insn_selection, check_width, reg_encoder_visitors, codegen_intrinsics).  
- To enable bytecode optimization, set gn arg **enable_bytecode_optimizer** (defines **ENABLE_BYTECODE_OPT**).

Typical build from ark root:

```bash
./ark.py x64.release --gn-args="enable_bytecode_optimizer=true"
```

### Test

- Unit tests are defined under **bytecode_optimizer/tests/** and depend on **libarkbytecodeopt** and generated abc files.  
- Run them via the project's test runner; use the test target that links the bytecode_optimizer (e.g. **bytecode_optimizer_tests** or a higher-level test suite).

---

## How to add a new feature (checklist)

### A) New or modified optimization pass

1. In **optimize_bytecode.cpp**, choose the right place in **RunOptimizations** and add **graph->RunPass<YourPass>()**.  
2. The pass must extend **compiler::Optimization** (or **compiler::Analysis**) and implement **RunImpl()** and **GetPassName()**.  
3. If it needs **BytecodeOptIrInterface** (e.g. string/method ID, literalarray), receive it via **RunOptimizations(..., iface)** and use it in the pass constructor or **RunImpl**.  
4. Some optimizations run only when **graph->IsDynamicMethod()** (see existing BranchElimination/ConstantPropagation conditions).

### B) Extend analysis results (BytecodeAnalysisResult)

1. Add storage and getters/setters to **BytecodeAnalysisResult**.  
2. If you add static read-only query APIs on **BytecodeAnalysisResults**, respect the existing mutex and "no concurrent writers" semantics.  
3. Fill the new fields in the **AnalysisBytecode** path (e.g. **AnalysisModuleRecordInfo**, **AnalysisModuleConstantValue**) or in a new analysis pass.

### C) Change codegen (IR → bytecode)

1. Most instructions are dispatched via generated visitors; new or changed instructions typically involve:  
   - **templates/codegen_visitors.inc.erb**, **insn_selection.\*.erb**, **codegen_intrinsics.cpp.erb**, and ISA description (e.g. **bytecode_optimizer_isapi.rb**).  
2. For a small number of opcodes, you can add static visitor handlers in **BytecodeGen** and wire them in **visitor.inc**.  
3. After changes, regenerate and run **codegen_test** and related tests.

### D) Options and configuration

1. Add the new option in **options.yaml** (name, type, default, description).  
2. Regenerate to get **generated/bytecodeopt_options_gen.h** and use **panda::bytecodeopt::options.Get...()** in code.  
3. To affect global compiler options, set them in **SetCompilerOptions()** (e.g. **compiler::options.SetCompilerMaxBytecodeSize(...)**).

---

## Critical pitfalls / "do not do this"

- **Do not** rely on analysis results from **BytecodeAnalysisResults** while another thread may be writing, without holding the appropriate synchronization or confirming "no writers". The read-only APIs document that they should be used when no writer is active.  
- **Do not** run the full optimization pipeline without setting **compiler::options** (e.g. MaxBytecodeSize, UseSafepoint); **OptimizeFunction** calls **SetCompilerOptions()** at the start.  
- **Do not** skip **RegEncoder** or **RegAlloc** in **RunOptimizations**; the resulting register layout may not satisfy VM frame constraints.  
- **Do not** mix allocators from different **compiler::Graph** instances in a single optimization; each method has its own Graph and ArenaAllocator.  
- **When changing codegen/reg_encoder generation**: update the corresponding `.erb` and ISA description and regenerate; otherwise generated and hand-written code can get out of sync.

---

## Where to look (quick reference)

| Goal | Location |
|------|----------|
| Entry API | **optimize_bytecode.h** |
| Analysis results and global access | **bytecode_analysis_results.h** |
| Per-method optimization order and skip logic | **RunOptimizations**, **OptimizeFunction**, **SkipFunction** in **optimize_bytecode.cpp** |
| IR and runtime abstraction | **ir_interface.h**, **runtime_adapter.h** |
| Constant propagation and lattice | **constant_propagation/constant_propagation.h**, **lattice_element.h**, **constant_value.h** |
| Registers and encoding | **reg_acc_alloc.h**, **reg_encoder.h**, **codegen.h** |
| Option definitions | **options.yaml**; generated header: **generated/bytecodeopt_options_gen.h** |
| Tests | **tests/*.cpp**, **tests/js/*.js**, and abc generation / test targets in **tests/BUILD.gn** |
