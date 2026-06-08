# Compiler Pipeline Debugging Knowledge

This file covers shared compiler pipeline triage used by JIT, OSR, AOT, and LLVM AOT work. Workflow contracts are covered by the workflow-specific knowledge files.

## Pipeline Model

| Stage | Start here | Risk |
|---|---|---|
| IR building | `static_core/compiler/optimizer/ir_builder/` | Wrong SSA shape affects every workflow |
| Analyses | `static_core/compiler/optimizer/analysis/` | Stale analysis assumptions can mislead later passes |
| Optimizations | `static_core/compiler/optimizer/optimizations/` | Shared passes can regress several modes at once |
| Register allocation | `static_core/compiler/optimizer/optimizations/regalloc/` | Location mistakes surface as codegen or ABI failures |
| Codegen | `static_core/compiler/optimizer/code_generator/` | Calling convention, slow path, and target assumptions affect runtime boundary |
| Code metadata | `static_core/compiler/code_info/` | Stack walking, deopt, OSR, and GC visibility depend on it |

## Pipeline Entry Points

`JITCompileMethod()` and `CompileInGraph()` in `static_core/compiler/compile_method.h` are common compilation entry points. They are templated on `TaskRunnerMode`, so first determine whether the task runs in-place or through background compilation.

The optimization pipeline is organized through `optimizer_run.h` and `optimizer/pipeline.cpp`. Non-optimizing mode still runs required structural passes before register allocation and codegen, while optimizing mode runs the full pass pipeline. OSR has workflow-specific constraints and historically differs from normal method compilation, so verify OSR behavior explicitly instead of assuming a normal JIT/AOT entry graph.

GraphChecker behavior is part of the diagnostic contract: debug builds can check after each pass, while `--compiler-check-final=true` only keeps the final post-codegen check. A final check is useful evidence, but it does not prove every intermediate pass invariant was preserved.

## Generated Source Map

| Source of truth | Generated / consumed behavior |
|---|---|
| `optimizer/ir/instructions.yaml` | IR opcodes, helpers, instruction metadata |
| bytecode-to-IR templates | generated instruction builder pieces such as `inst_builder_gen.cpp` |
| plugin pipeline hooks | language plugin pass insertion such as generated `create_pipeline.inl` |
| intrinsic templates and metadata | intrinsic IR build, peephole, expansion, and codegen hooks |
| compiler YAML/templates | compiler options and logger components such as `compiler_options_gen.h` |

When behavior looks inconsistent, first determine whether the handwritten file or generated output is the source of truth.

## Diagnostic Routing

| Scenario / Keywords | Read first |
|---|---|
| compiler flags, dumps, GraphChecker, tracing | `static_core/compiler/docs/compiler_doc.md` |
| compilation entry, in-place vs background runner | `static_core/compiler/docs/compilation_start.md` |
| wrong IR shape / bytecode lowering | `static_core/docs/ir_format.md`, `static_core/compiler/docs/ir_builder.md` |
| wrong machine code / slow path / call convention | `static_core/compiler/docs/codegen_doc.md` |
| pass-specific behavior | `static_core/compiler/docs/README.md` and the matching pass doc |
| performance regression | `static_core/compiler/docs/performance_workflows.md` |
| compiler-wide historical notes and caveats | `static_core/docs/compiler_intro_current.md` |

## Build And Test Commands

Keep these commands here because they are the usual first proof for compiler-local changes:

```sh
ninja arkcompiler
ninja ark_aot
ninja ark_aotdump
ninja ark_aptool
```

```sh
ninja compiler_unit_tests && ./bin-gtests/compiler_unit_tests
ninja compiler_codegen_tests && ./bin-gtests/compiler_codegen_tests
ninja encoder_unit_tests && ./bin-gtests/encoder_unit_tests
ninja core_checked_tests
./bin-gtests/compiler_unit_tests --gtest_filter="SomeTest.*"
```

The GN target for the compiler library is `compiler:libarktscompiler`.

## Debugging Starter Set

Use `static_core/compiler/docs/compiler_doc.md` as the full flag catalog. The common first pass is:

```sh
--compiler-regex='Class::method'
--compiler-dump --compiler-dump:folder=./ir_dump
--compiler-disasm-dump:single-file
--compiler-log=inlining,licm-opt,peephole --log-components=compiler --log-level=debug
```

Useful side tools:

- `ark_aotdump` inspects `.an` files and can dump disassembly for matching methods.
- `ark_aptool dump --ap-path=profile.ap --output=profile.yaml` dumps profile content; add `--abc-path` or `--abc-dir` when resolved class/method names are needed.
- `--paoc-generate-symbols` keeps AOT method symbols for external `perf` or flamegraph workflows.
- `compiler/tools/debug/jit_writer.*` contains GDB/JIT debug info support.
- `compiler/tools/benchmark_coverage.sh` and `compiler/cmake/benchmark_coverage.cmake` exercise AOT/JIT/OSR benchmark coverage.
- `compiler/tools/ir_builder_coverage.sh` reports IR builder instruction coverage against unit tests.

Tracing caveat: `--compiler-enable-tracing` is useful for pass timelines, but tracing hooks are compiled only in non-`NDEBUG` builds.

## Boundaries

- Do not debug a workflow failure only at the final crash site; classify whether the first wrong state is IR, analysis, optimization, regalloc, codegen, metadata, or runtime boundary.
- Do not trust a generated file as the owner until the source template or YAML has been checked.
- Do not use `--compiler-check-final=true` as proof that intermediate pass invariants are safe.
- Do not remove dumps, events, logs, or GraphChecker coverage to hide an unstable pass.

## Modification Checklist

Before editing, confirm:

- The earliest wrong pipeline stage is named, not only the final failure site.
- Source YAML/templates and generated output are both considered when generation is involved.
- JIT, OSR, AOT, and LLVM AOT impact is listed for shared passes.
- Code-info, stack map, deopt, or GC visibility evidence is included when runtime metadata changes.

## Code and Test Anchors

- IR/pass behavior: compiler gtest or checked IR dump.
- Codegen behavior: compiler codegen test, checked disasm, or target runtime execution.
- Generated source behavior: generator source plus regenerated output in build artifacts.
- Runtime boundary behavior: runtime/compiler execution plus stack, deopt, exception, or GC-sensitive coverage.
