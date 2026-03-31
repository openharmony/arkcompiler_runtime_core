---
name: ark-perf-lab
description: Investigate Ark static_core performance regressions with current benchmark layers, explicit runtime mode selection, GC control, IR and disassembly inspection, AOT symbol generation, and external profiler handoff. Use when comparing `int`, `jit`, and `aot`, running VMB or ETS micro-benchmarks, or preparing a clean profiling workflow.
---

# Ark Perf Lab

## When to use

- Measure a regression or win in `int`, `jit`, or `aot`.
- Choose between VMB and ETS micro-benchmarks.
- Separate GC noise from a real compiler delta.
- Prepare symbolized AOT output or a clean external profiling run.

## Current sources of truth

- `../../compiler/docs/performance_workflows.md` for the maintained performance investigation loop.
- `../../tests/AGENTS.md` for benchmark and validation layer routing.
- `../../tests/vm-benchmarks/readme.md` for VMB workflows and reporting.
- `../../plugins/ets/tests/scripts/micro-benchmarks/README.md` for focused ETS `int`/`jit`/`aot` runs.
- `../../runtime/tooling/sampler/docs/cpu_profiler.md` for the runtime CPU profiler entry points.
- `../../runtime/options.yaml` for runtime mode and GC flags.
- `../../compiler/tools/paoc/paoc.yaml` for `--paoc-generate-symbols=true`.
- `../../compiler/compiler.yaml` for method-local compiler inspection flags.

## Workflow

1. Choose the benchmark layer.

- Use `tests/vm-benchmarks/` for broader benchmark suites and comparison reports.
- Use ETS micro-benchmarks for focused `int`, `jit`, and `aot` comparisons on one workload.
- Do not trust ad-hoc handwritten loops when a harness already exists.

2. Stabilize runtime mode and environment.

- Keep `int`, `jit`, and `aot` runs separate.
- Pass explicit `--interpreter-type=cpp|irtoc|llvm` for reproducibility.
- Keep build type, architecture, and runtime shape fixed across comparisons.

3. Control GC before judging the compiler.

- Use runtime GC flags deliberately.
- Turn on GC statistics or logs before blaming an optimization change.
- Treat GC postponement APIs as experiment controls, not as default benchmark settings.

4. Inspect compiler output before external profiling.

- Use `--compiler-regex`, `--compiler-dump`, and `--compiler-disasm-dump:single-file` on the hot method.
- For AOT profiling, use `--paoc-generate-symbols=true` when symbols matter.

5. Only then move to external profiling.

- Use Linux `perf`, the runtime CPU profiler, or another external tool after the mode and workload are stable.
- If the investigation becomes `.ap`-driven, hand off to `../ark-aot-pgo/SKILL.md`.

### VMB example

```bash
cd ./tests/vm-benchmarks
export PANDA_BUILD=${BUILD}
vmb all --platform=arkts_host --src-langs=ets --langs=ets --mode=jit ./examples/benchmarks/
```

Use `--aot-skip-libs` only when you intentionally control stdlib freshness; stale AOT-compiled stdlib can skew the
measurement.

### ETS micro-benchmark example

```bash
python3 ./plugins/ets/tests/scripts/micro-benchmarks/run_micro_benchmarks.py \
  --bindir ${BUILD}/bin \
  --mode jit \
  --interpreter-type llvm \
  --test-name MyBenchmark
```

### AOT compile with symbols for profiling

```bash
${BUILD}/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --paoc-output=app.an \
  --paoc-generate-symbols=true
```

## Common pitfalls

- Do not mix `int`, `jit`, and `aot` numbers in one uncontrolled run.
- Do not compare different interpreter implementations unless that is the actual question.
- Do not skip GC visibility and then over-attribute the delta to compiler output.
- Do not jump to flamegraphs before checking IR or disassembly.
- Do not duplicate `.ap` workflow detail here; use `../ark-aot-pgo/SKILL.md` when profiles become central.

## Escalate / handoff

- If you need a raw reproducible mode matrix first, continue in `../ark-jit-aot-repro/SKILL.md`.
- If you need compiler-local hot-method inspection, continue in `../ark-ir-disasm-triage/SKILL.md`.
- If the investigation becomes profile-guided AOT, continue in `../ark-aot-pgo/SKILL.md`.
