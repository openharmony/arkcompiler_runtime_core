# Compiler

Core optimizing compiler for ArkTS-Sta. It lowers `.abc` bytecode into SSA IR, runs analyses and optimizations, then
emits native code for JIT, OSR, AOT, and optional LLVM AOT workflows.

## Compiler Modes

- `JIT` - runtime compilation of hot methods, installing code into the code cache
- `OSR` - loop-entry compilation for hot running methods; runtime entry is in `runtime/osr.cpp`
- `AOT` - `ark_aot --paoc-mode=aot` emits `.an` files
- `LLVM AOT` - `ark_aot --paoc-mode=llvm` uses the LLVM backend when enabled
- `paoc` `jit` and `osr` modes - run graph building and optimization/codegen validation without writing `.an` output

Runtime-side orchestration lives outside this directory in `runtime/compiler.cpp`, `runtime/compiler_queue_*`,
`runtime/compiler_*worker.*`, and `runtime/osr.cpp`.

## Directory Structure

- `optimizer/ir/` - IR definitions: `Graph` (compilation unit), `BasicBlock` (CFG node), `Inst` hierarchy, dumps, runtime interface
- `optimizer/ir_builder/` - Bytecode to SSA IR translation (IrBuilder)
- `optimizer/analysis/` - Analyses such as dominators, loops, liveness, alias analysis, bounds analysis, hotness propagation
- `optimizer/optimizations/` - Optimization passes including lowering, inlining, peepholes, LICM, LSE, escape analysis, checks elimination, branch elimination, loop transforms, string optimizations
- `optimizer/optimizations/regalloc/` - Register allocation: linear scan, graph coloring, interference graph, verifier
- `optimizer/code_generator/` - IR to native code generation, slow paths, calling conventions, target backends under `target/`
- `aot/` - AOT compilation infrastructure and `.an` file format
- `code_info/` - Metadata used by stack walking, deoptimization, and OSR
- `tests/` - Compiler gtests for IR builder, analyses, optimizations, regalloc, AOT, profiling, OSR, codegen, encoders
- `tools/paoc/` - Offline compilation tool (`ark_aot`)
- `tools/aotdump/` - Inspect `.an` files and dump code/disassembly
- `tools/aptool/` - Inspect `.ap` profile files
- `tools/debug/` - JIT debug info helpers
- `docs/` - Design docs for compiler pipeline, passes, codegen, AOT, PGO, IR builder, compilation start
- `compiler.yaml` - Compiler options definition, generates `compiler_options_gen.h` at build time

## Compilation Flow

Entry points are `JITCompileMethod()` and `CompileInGraph()` in `compile_method.h`, templated on `TaskRunnerMode` to
distinguish in-place and background compilation.

- Graph construction starts in `optimizer/ir_builder/`
- The optimization pipeline is implemented in `optimizer_run.h` and `optimizer/pipeline.cpp`
- Language plugins extend it through generated `create_pipeline.inl`
- Non-optimizing mode still runs `TryCatchResolving` and `MonitorAnalysis` before `RegAlloc -> CodeGen`
- Optimizing mode runs the full pass pipeline from `optimizer/pipeline.cpp`
- OSR mode currently skips `Inlining`
- Debug builds run `GraphChecker` after each pass; `--compiler-check-final=true` keeps only the final post-CodeGen check

For task-runner details and async/in-place orchestration examples, see `docs/compilation_start.md`.

## Task Map

When the problem is compiler-related, map it to the nearest subsystem before editing:

- wrong SSA / pass behavior - start in `optimizer/ir_builder/`, `optimizer/analysis/`, `optimizer/optimizations/`, `pipeline.*`, then use IR dumps
- wrong machine code / ABI mismatch - start in `optimizer/code_generator/`, `optimizer/code_generator/target/`, compiler callconv tests, and runtime bridge tests
- deoptimization / OSR / stack-map bug - start in `code_info/` plus the owning runtime guide in `../runtime/AGENTS.md`; use `../docs/code_metainfo.md`, `../docs/deoptimization.md`, and `../docs/on-stack-replacement.md` as focused references
- runtime call / bridge / entrypoint bug - start in `optimizer/code_generator/` and the owning runtime guide in `../runtime/AGENTS.md`; the main runtime-side files are `runtime/entrypoints/`, `runtime/bridge/`, and `../docs/runtime-compiled_code-interaction.md`
- intrinsic / fastpath work - start in generated intrinsic hooks under `optimizer/templates/intrinsics/`, then follow into `irtoc/`, `runtime/compiler.cpp`, and the language plugin
- profile-guided or hotness issue - start in `../runtime/jit/AGENTS.md`, `tools/aptool/`, `tools/paoc/`, and `docs/aot_pgo.md`
- performance regression - combine compiler dumps/logs with `docs/performance_workflows.md` plus the owning benchmark or runtime tooling docs

## Generated Sources and Extension Points

Compiler development often crosses generated files:

- `optimizer/ir/instructions.yaml` + ERB templates generate IR opcodes and helpers
- `inst_builder_gen.cpp` is generated for bytecode-to-IR lowering
- `create_pipeline.inl` is generated and used to merge plugin passes into the core pipeline
- intrinsics-related `.inl` files are generated from templates and runtime/compiler metadata
- `compiler_options_gen.h` and compiler logger components are generated from YAML/templates

When debugging surprising compiler behavior, verify whether the source of truth is the handwritten file or generated output.

## Build

```bash
# CMake main targets
ninja arkcompiler       # compiler library
ninja ark_aot           # AOT compiler
ninja ark_aotdump       # inspect .an files
ninja ark_aptool        # inspect .ap profiles
```

GN target: `compiler:libarktscompiler`.

Build depends on Ruby/ERB-based generation for IR opcodes, inst builder code, compiler options, logger components, and
intrinsics-related files.

## Testing

```bash
ninja compiler_unit_tests && ./bin-gtests/compiler_unit_tests
ninja compiler_codegen_tests && ./bin-gtests/compiler_codegen_tests
ninja encoder_unit_tests && ./bin-gtests/encoder_unit_tests
ninja core_checked_tests
# filter
./bin-gtests/compiler_unit_tests --gtest_filter="SomeTest.*"
```

Use the owning guide for broader validation instead of duplicating those matrices here:

- `compiler/tests/` - compiler-owned gtests for IR builder, optimizations, regalloc, AOT, profiling, and codegen
- `../tests/AGENTS.md` - checked tests, URunner workflows, release/arch gating, and flaky repro discipline
- `../tests/tests-u-runner/readme.md` and `../tests/tests-u-runner-2/README.md` - URunner v1/v2 entry points for new
  ETS checked coverage and CI-like reproductions
- `../runtime/AGENTS.md` - stack-walker, bridge, and other runtime/compiler boundary gtests
- `../plugins/ets/compiler/AGENTS.md` - ETS-specific compiler validation layers

## Debugging

Use `docs/compiler_doc.md` as the canonical flag catalog and dump-format reference. The most common starting set is:

```bash
--compiler-regex='Class::method'                                # compile only matching methods
--compiler-dump --compiler-dump:folder=./ir_dump                # dump IR after passes
--compiler-disasm-dump:single-file                              # dump generated assembly into disasm.txt
--compiler-log=inlining,licm-opt,peephole --log-components=compiler --log-level=debug
```

Useful tooling outside normal compiler flags:

- `ark_aotdump` inspects `.an` files and can dump disassembly for matching methods
- `ark_aptool dump --ap-path=profile.ap --output=profile.yaml` dumps profile content to YAML; add `--abc-path` or `--abc-dir` if you need resolved class/method names
- `--paoc-generate-symbols` is the current AOT-side flag to keep method symbols for external `perf` / flamegraph workflows
- `compiler/tools/debug/jit_writer.*` contains GDB/JIT debug info support
- `compiler/tools/benchmark_coverage.sh` and `compiler/cmake/benchmark_coverage.cmake` exercise AOT/JIT/OSR benchmark coverage
- `compiler/tools/ir_builder_coverage.sh` reports IR builder instruction coverage against unit tests

Tracing caveat:

- `--compiler-enable-tracing` is useful for pass timelines, but the hooks are compiled only in non-`NDEBUG` builds

For less common flags such as `--compiler-check-final`, `--compiler-enable-events`, `--compiler-dump-stats-csv`,
`--compiler-visualizer-dump`, or dump suboptions, see `docs/compiler_doc.md`.

## Fast Paths and Intrinsics

- Compiler-generated fastpath calls go through `Codegen::CallFastPath`; slow paths use runtime calls
- Fastpath availability depends on runtime support and GC constraints, so compiler work in this area usually also touches `runtime/compiler.cpp` and `irtoc/`
- Language-specific intrinsic lowering/codegen is injected by plugins through generated `.inl` hooks and `RuntimeInterface`
- Follow the repo-level `irtoc` policy in `../AGENTS.md` and the detailed irtoc guidance in `../irtoc/AGENTS.md`
- Compiler-specific rule of thumb: compare `irtoc` against inlineable managed code and C++ runtime implementations before adding a new fastpath entrypoint, then validate the win on the target architecture and build mode

## Plugin Extension

Language plugins (e.g. ETS) extend the compiler in three ways:

- `create_pipeline.inl` - inject custom optimization passes
- `intrinsics_*.inl` files - inject language-specific intrinsic handling
- Subclass `RuntimeInterface` (`optimizer/ir/runtime_interface.h`) - provide language-specific runtime queries

## Documentation

`docs/README.md` is the compiler-owned documentation index. Keep detailed pass, pipeline, codegen, and paoc material there
instead of repeating the catalog in multiple guides.

Neighbor guides for topics owned outside `compiler/`:

- `../runtime/AGENTS.md` - runtime/compiler integration, deopt, OSR, stack walking, bridges
- `../runtime/jit/AGENTS.md` - runtime profiling data and `.ap` persistence
- `../irtoc/AGENTS.md` - irtoc pipeline and fastpath guidance
- `../tests/AGENTS.md` - checked, runner, and compiler-facing validation workflows
