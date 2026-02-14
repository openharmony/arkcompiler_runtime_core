# Compiler

JIT/AOT compiler that compiles `.abc` bytecode into native machine code (`.an` files or in-memory JIT code).

## Directory Structure

- `optimizer/ir/` - IR definitions: Graph (compilation unit), BasicBlock (CFG node), Inst (instruction base class). Instructions defined in `instructions.yaml`, C++ code generated via Ruby ERB templates
- `optimizer/ir_builder/` - Bytecode to SSA IR translation (IrBuilder)
- `optimizer/optimizations/` - Optimization passes including peepholes, inlining, lse, licm, escape, checks_elimination, branch_elimination, loop_unroll, etc.
- `optimizer/optimizations/regalloc/` - Register allocation (linear_scan and graph_coloring implementations)
- `optimizer/analysis/` - Analysis passes: dominators_tree, loop_analyzer, liveness_analyzer, alias_analysis, bounds_analysis, etc.
- `optimizer/code_generator/` - IR to native code generation (Codegen), backends under `target/`: aarch64, amd64, aarch32, x86
- `aot/` - AOT compilation infrastructure and `.an` file format
- `tools/paoc/` - Offline compilation tool (Paoc class), build artifact is `ark_aot`. AOT/LLVM modes produce `.an` files; JIT/OSR modes only run the compilation pipeline without producing code, used for testing
- `compiler.yaml` - Compiler options definition, generates `compiler_options_gen.h` at build time

## Compilation Flow

Entry points are `JITCompileMethod()` and `CompileInGraph()` in `compile_method.h`, templated on `TaskRunnerMode` to distinguish sync/async modes. The optimization pipeline is defined in `pipeline.h/cpp`, extensible by language plugins via `create_pipeline.inl` (generated file). Non-optimizing mode only runs TryCatchResolving -> MonitorAnalysis -> RegAlloc -> CodeGen; optimizing mode runs the full pipeline. Debug builds run GraphChecker after each pass to verify IR correctness.

## Build

```bash
# CMake main targets
ninja arkcompiler       # compiler library
ninja ark_aot           # AOT compiler
```

GN target: `compiler:libarktscompiler`.

Build depends on Ruby ERB template generation: `inst_builder_gen.cpp` (bytecode to IR), `opcodes.h` (instruction definitions), `compiler_options_gen.h` (options), etc.

## Testing

```bash
ninja compiler_unit_tests && ./bin-gtests/compiler_unit_tests
ninja compiler_codegen_tests && ./bin-gtests/compiler_codegen_tests
ninja encoder_unit_tests && ./bin-gtests/encoder_unit_tests
# filter
./bin-gtests/compiler_unit_tests --gtest_filter="SomeTest.*"
```

## Debugging

```bash
--compiler-dump --compiler-dump:folder=./dumps   # dump IR after each pass
--compiler-disasm-dump                           # dump generated assembly
--compiler-regex="ClassName::method"             # compile only matching methods
```

## Plugin Extension

Language plugins (e.g. ETS) extend the compiler in three ways:
- `create_pipeline.inl` - inject custom optimization passes
- `intrinsics_*.inl` files - inject language-specific intrinsic handling
- Subclass `RuntimeInterface` (`optimizer/ir/runtime_interface.h`) - provide language-specific runtime queries

## Documentation

~50 markdown files under `docs/` covering major optimization passes, register allocation, codegen, AOT, etc.
