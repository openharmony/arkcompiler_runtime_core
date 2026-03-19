---
name: ark-jit-aot-repro
description: Reproduce Ark static_core issues across interpreter, JIT, AOT, and offline paoc flows. Use when you need exact `ark` or `ark_aot` commands, explicit `--interpreter-type`, `--compiler-regex` minimization, current `--paoc-*` flags, or a fast choice between interpreter-only, force-JIT, runtime AOT, and offline paoc validation.
---

# Ark JIT/AOT Repro

## When to use

- Reproduce a crash, wrong result, or miscompile in one execution mode only.
- Compare interpreter, force-JIT, and AOT behavior with exact commands.
- Minimize a broad failure down to one method with `--compiler-regex`.
- Validate whether the problem still reproduces in offline `ark_aot --paoc-mode=jit|osr|aot`.

## Current sources of truth

- `../../runtime/options.yaml` for runtime flags such as `--interpreter-type`, `--compiler-enable-jit`, `--no-async-jit`, `--aot-files`, and `--enable-an`.
- `../../compiler/tools/paoc/paoc.yaml` for current `ark_aot` flags such as `--paoc-panda-files`, `--paoc-output`, `--paoc-location`, and `--paoc-mode`.
- `../../compiler/compiler.yaml` for compiler-local dump and logging flags such as `--compiler-regex`, `--compiler-dump`, and `--compiler-disasm-dump:single-file`.
- `../../README.md` for baseline `ark` and `ark_aot` command shapes.
- `../../docs/flaky_debugging.md` for current minimization and repro discipline.
- `../../docs/aot.md` for runtime-visible AOT behavior and path matching.
- `../../compiler/docs/paoc.md` for current `ark_aot` modes and option groups.
- `../../runtime/AGENTS.md` for force-JIT and runtime-side caveats.

## Workflow

1. Start by choosing one lane and keeping the rest disabled.

### Interpreter-only repro

- Use this first when you need a clean non-JIT baseline.
- Always pass `--interpreter-type=cpp|irtoc|llvm` explicitly for reproducibility.

```bash
${BUILD}/bin/ark \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --interpreter-type=llvm \
  --compiler-enable-jit=false \
  app.abc app.ETSGLOBAL::main
```

### Force-JIT repro

- Use this when the bug appears only after code generation or installation.
- Pair `--no-async-jit=true` with hotness control; the common force-JIT shape is `--compiler-hotness-threshold=0`.
- Narrow early with `--compiler-regex='Class::method'`.

```bash
${BUILD}/bin/ark \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --interpreter-type=llvm \
  --compiler-enable-jit=true \
  --compiler-hotness-threshold=0 \
  --no-async-jit=true \
  --compiler-regex='App::hotMethod' \
  app.abc app.ETSGLOBAL::main
```

### Runtime AOT run

- Compile first with current `--paoc-*` flags, then run with `--aot-files=...`.
- Keep `--boot-panda-files` aligned between compile and run.
- Use `--paoc-location` and `--paoc-boot-location` when runtime path equality matters.

```bash
${BUILD}/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --paoc-output=app.an \
  --paoc-location="$PWD"

${BUILD}/bin/ark \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --interpreter-type=llvm \
  --aot-files=app.an \
  app.abc app.ETSGLOBAL::main
```

### Offline paoc validation

- Use this to validate pipeline shape without a full runtime run.
- `--paoc-mode=jit` and `--paoc-mode=osr` are useful when the issue looks compiler-side.
- Keep method scope small with `--compiler-regex`.

```bash
${BUILD}/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --paoc-mode=jit \
  --compiler-regex='App::hotMethod'
```

2. Once one lane reproduces, shrink the scope.

- Re-run a single entry point.
- Add `--compiler-regex` as soon as one method is suspicious.
- For compiler-shaped failures, hand off to `../ark-ir-disasm-triage/SKILL.md`.

## Common pitfalls

- Do not rely on implicit interpreter defaults; the runtime may downgrade them.
- Do not combine force-JIT (`--no-async-jit=true`) with loaded AOT files.
- Do not mix old paoc names such as `--output`, `--location`, or `--panda-files`; use `--paoc-output`, `--paoc-location`, and `--paoc-panda-files`.
- Do not let compile-time and run-time `--boot-panda-files` diverge for AOT.
- Do not ignore path metadata when the AOT file is produced on one path and consumed from another.

## Escalate / handoff

- If the problem reproduces only after code installation or frame transition, continue in `../../runtime/AGENTS.md`.
- If the next step is IR, pass, or disassembly inspection, continue in `../ark-ir-disasm-triage/SKILL.md`.
- If the failure came from checked or URunner infrastructure, continue in `../ark-checked-urunner-repro/SKILL.md`.
