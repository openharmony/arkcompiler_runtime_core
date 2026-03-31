---
name: ark-aot-pgo
description: Work with Ark static_core AOT PGO flows using `.ap` profiles, runtime saver flags, `ark_aptool dump`, and `ark_aot --paoc-use-profile:path=...`. Use when collecting profile data, validating saved profiles, reusing them in AOT compilation, or debugging class-context mismatch and current AOT PGO limitations.
---

# Ark AOT PGO

## When to use

- Collect `.ap` data from a workload and feed it back into `ark_aot`.
- Check whether a saved profile is usable before trusting a PGO result.
- Debug why `--paoc-use-profile:path=...` changed nothing or was rejected.
- Inspect class-context or inline-cache limitations in current AOT PGO.

## Current sources of truth

- `../../runtime/options.yaml` for `--profile-output`, `--profilesaver-enabled`, `--incremental-profilesaver-enabled`, `--compiler-profiling-threshold`, and `--profile-branches`.
- `../../compiler/tools/paoc/paoc.yaml` for `--paoc-use-profile:path=...` and other current paoc flags.
- `../../compiler/docs/aot_pgo.md` for current compiler-side PGO behavior and limitations.
- `../../docs/bytecode_profiling.md` for the runtime profiling flow and `.ap` overview.
- `../../docs/aptool.md` for `ark_aptool` usage and output formats; this workflow specifically relies on `ark_aptool dump`.
- `../../runtime/jit/AGENTS.md` and `../../runtime/jit/libprofile/` for profile ownership, caveats, and current on-disk
  layout.
- `../../compiler/compiler.yaml` for `--compiler-regex` and dump flags used to verify the changed compilation.

## Workflow

1. Collect a profile from the real workload.

- `--profile-output` alone is not enough; pair it with `--profilesaver-enabled=true`.
- Use `--incremental-profilesaver-enabled=true` only when background saving is useful.
- Only add `--compiler-profiling-threshold` or `--profile-branches=true` when the workload needs them.
- If the setup disables profiling, pass `--profiler-enabled=true` explicitly.

```bash
${BUILD}/bin/ark \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --interpreter-type=llvm \
  --profilesaver-enabled=true \
  --profile-output=workload.ap \
  app.abc app.ETSGLOBAL::main
```

2. Inspect the saved profile before reusing it.

- Require `ark_aptool dump` before concluding the profile is useful.
- Add `--abc-path` when you need readable class and method names.

```bash
${BUILD}/bin/ark_aptool dump \
  --ap-path workload.ap \
  --output workload.yaml \
  --abc-path app.abc
```

3. Reuse the profile in AOT.

- Use the current form `--paoc-use-profile:path=<profile.ap>[,force]`.
- Keep the same boot/runtime context that produced the workload.
- Load the same application panda files through `--panda-files` before the profile is applied.
- If `--paoc-panda-files` contains more than one file, mirror the same set in `--panda-files` with `:` separators.

```bash
${BUILD}/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --panda-files=app.abc \
  --paoc-output=app.an \
  --paoc-use-profile:path=workload.ap
```

4. Verify the changed compilation.

- Re-run with `--compiler-regex`, `--compiler-dump`, or `--compiler-disasm-dump:single-file`.
- Compare one profiled method at a time.

## Common pitfalls

- Do not assume `profile.ap` is meaningful until `ark_aptool dump` confirms the expected workload is present.
- Do not forget that class-context mismatch rejects the profile.
- Do not forget to mirror profiled application files into `--panda-files` when reusing a profile.
- Do not expect AOT PGO to restore JIT-style CHA behavior.
- Do not expect AOT PGO to bypass `--compiler-inline-external-methods-aot` or the normal AOT external-inlining checks.
- Do not use historical paoc flag names when adding the profile to `ark_aot`.

## Escalate / handoff

- If the problem becomes a general JIT/AOT reproduction question, continue in `../ark-jit-aot-repro/SKILL.md`.
- If you now need IR and disassembly comparison on one profiled method, continue in `../ark-ir-disasm-triage/SKILL.md`.
- If the task is broader benchmark investigation, continue in `../ark-perf-lab/SKILL.md`.
