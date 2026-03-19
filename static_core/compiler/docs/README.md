# Compiler Documentation Map

`compiler/docs/` owns compiler-specific design notes: pipeline shape, passes, code generation, and `ark_aot`
workflows. Use `../AGENTS.md` for active engineering tasks and this file only as the index for material that lives in
`compiler/docs/`.

Docs owned by `runtime/`, `irtoc/`, `tests/`, or the top-level `docs/` directory are intentionally not duplicated
here. When the topic crosses a subsystem boundary, jump to the owning guide instead of treating `compiler/docs/` as the
repository-wide documentation hub.

## Compiler-Owned Docs

### Modes and Pipeline

- `compiler_doc.md` - compiler flags, IR/disasm dumps, logging, GraphChecker workflow
- `compilation_start.md` - current in-place vs background task-runner usage
- `ir_builder.md` - bytecode-to-SSA construction, save states, OSR-sensitive IR building
- `paoc.md` - current `ark_aot` / paoc modes and option map
- `aot_pgo.md` - compiler-facing AOT PGO workflow
- `performance_workflows.md` - benchmark, GC-control, `perf`, and AOT PGO investigation workflow

### Codegen and Compiler-Side Runtime Boundary

- `codegen_doc.md` - backend structure, slow paths, and disassembly workflow
- `bridges.md` - SaveState Bridges optimization inside compiler IR
- `interface_inline_cache.md` - compiler-side inline-cache behavior

### Optimizations and Analyses

Pass-specific notes live here and usually map 1:1 to files in `compiler/optimizer/analysis/` and
`compiler/optimizer/optimizations/`:

- `inlining.md`, `branch_elimination_doc.md`, `check_elimination_doc.md`, `deoptimize_elimination_doc.md`
- `licm_doc.md`, `loop_unrolling.md`, `loop_peeling.md`, `loop_unswitch_doc.md`
- `escape_analysis.md`, `lse_doc.md`, `memory_barriers_doc.md`, `memory_coalescing_doc.md`
- `reg_alloc_linear_scan_doc.md`, `reg_alloc_graph_coloring_doc.md`

## Neighbor Docs Owned Outside `compiler/`

Use the owning guide when the main question is outside compiler-owned documentation:

- runtime integration, stack maps, deopt, and OSR: `../../runtime/AGENTS.md`, `../../docs/code_metainfo.md`,
  `../../docs/deoptimization.md`, `../../docs/on-stack-replacement.md`
- irtoc and fastpath pipeline: `../../irtoc/AGENTS.md`, `../../docs/irtoc.md`
- profiling data and `ark_aptool`: `../../runtime/jit/AGENTS.md`, `../../docs/bytecode_profiling.md`,
  `../../docs/aptool.md`
- checked tests, runners, and flaky-debug workflows: `../../tests/AGENTS.md`, `../../docs/flaky_debugging.md`
- compiler-wide notes and caveats: `../../docs/compiler_intro_current.md`

## Validation and Perf Entry Points

Compiler-owned docs should be paired with the test or benchmark layer that owns the workflow:

- compiler gtests: `../tests/`
- runtime/compiler boundary gtests: `../../runtime/tests/`
- checked and runner workflows: `../../tests/checked/README.md`, `../../tests/tests-u-runner/`,
  `../../tests/tests-u-runner-2/`
- benchmarks and perf suites: `../tools/benchmark_coverage.sh`, `../../tests/vm-benchmarks/`,
  `../../tests/benchmarks/`, `../../plugins/ets/tests/scripts/micro-benchmarks/README.md`
