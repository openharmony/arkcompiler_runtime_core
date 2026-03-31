# Performance Workflows

This note describes the current performance-analysis workflow for `static_core`. Use it together with
`tests/vm-benchmarks/`, `compiler_doc.md`, and the profiling docs.

## Choose the Right Benchmark Layer

The current tree has two main benchmark entry points:

- `tests/vm-benchmarks/` for VMB-based runs, including warmup, reporting, and cross-platform workflows
- `plugins/ets/tests/scripts/micro-benchmarks/README.md` for focused ETS micro-benchmark runs across `int`, `jit`, and
  `aot`

When possible:

- keep the measured result observable so the optimizer cannot remove the work
- use the benchmark harness instead of handwritten one-off loops
- compare the same build type, architecture, GC mode, and interpreter mode

For freestyle VMB tests, the current contract is still:

`Benchmark result: <TestName> <time in ns>`

## Stabilize the Environment

Performance differences are often smaller than environment noise. For current local workflows:

- use explicit runtime modes such as `--interpreter-type=cpp|irtoc|llvm`
- keep JIT, AOT, and interpreter runs separated
- warm up benchmarks before measuring
- fix CPU affinity when the platform or harness supports it

VMB already exposes warmup and measurement controls such as:

- `--warmup-iters`
- `--measure-iters`
- `--warmup-time`
- `--iter-time`
- `--sys-gc-pause`

Do not generalize host-only wins to device conclusions without rechecking on the real target.

## Control GC Before Drawing Conclusions

GC noise is still one of the main sources of misleading numbers.

Current runtime knobs include:

- `--gc-type=epsilon`
- `--gc-type=epsilon-g1`
- `--gc-trigger-type=trigger-heap-occupancy`
- `--max-trigger-percent=<n>`
- `--print-gc-statistics=true`
- `--print-memory-statistics=true`
- `--log-level=info --log-components=gc`
- `--log-detailed-gc-info-enabled=true`

For ETS-specific experiments, current stdlib APIs also expose:

- `GC.startGC(...)`
- `GC.waitForFinishGC(gcId)`
- `GC.postponeGCStart()`
- `GC.postponeGCEnd()`

Treat these APIs as experiment controls, not as benchmark defaults. If a performance claim depends on postponing GC,
or forcing a collection boundary explicitly, document that explicitly.

## Use IR and Disassembly Early

For compiler-driven regressions, current repo-supported analysis still starts with IR and disassembly:

- `--compiler-regex='Class::method'`
- `--compiler-dump`
- `--compiler-dump:final`
- `--compiler-dump:bytecode`
- `--compiler-dump:life-intervals`
- `--compiler-disasm-dump:single-file`

For AOT-specific work:

- `ark_aot --paoc-panda-files=... --paoc-mode=aot`
- `ark_aotdump`
- `--paoc-generate-symbols=true` when you need symbolized AOT output

Typical questions to answer in the dump before doing deeper profiling:

- did the expected call devirtualize?
- did the expected helper inline?
- did constant propagation or folding actually remove the value or load you care about?
- did a presumed slow path remain in the cold block, or is that path actually the one taken most of the time?
- did a register-allocation or codegen change inflate code size or spills?

## AOT PGO

This workflow uses `.ap` profiles, but the data model, save/load flow, and current compiler limitations are owned by
`aot_pgo.md`.

Use this document only for the perf-facing part of the loop:

- confirm that the benchmark delta is real before introducing PGO into the investigation
- dump the profile with `ark_aptool` to verify that the expected workload was recorded
- then switch to `aot_pgo.md` for compiler consumption details and current AOT restrictions

## External Profilers

The repository does not ship a maintained flamegraph script set, but Linux `perf` remains a valid external workflow.
For AOT investigations, make sure symbols are actually available:

- build with frame pointers if your profiling workflow needs them
- use `--paoc-generate-symbols=true` for AOT code when symbolized output matters
- keep host and device symbol environments separate

Treat host flamegraphs as supporting evidence, not as the final verdict for device behavior.

## Minimal Investigation Loop

1. reproduce on one benchmark and one configuration only
2. confirm the result is stable enough to compare
3. inspect IR and disassembly
4. confirm GC is not the real source of the delta
5. only then move to external profiling or controlled source changes

## Related Docs

- `README.md`
- `compiler_doc.md`
- `paoc.md`
- `aot_pgo.md`
- `../../docs/aptool.md`
- `../../docs/bytecode_profiling.md`
- `../../docs/flaky_debugging.md`
