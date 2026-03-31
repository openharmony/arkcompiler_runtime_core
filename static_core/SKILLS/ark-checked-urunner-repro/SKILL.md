---
name: ark-checked-urunner-repro
description: Reproduce Ark static_core failures through checked tests or URunner v1/v2. Use when narrowing CI regressions, capturing exact `checker.rb`, `ark`, or `ark_aot` commands, authoring `RUN` or `RUN_PAOC` style repros, or deciding whether a bug belongs in checked, URunner, or raw tool-level reproduction.
---

# Ark Checked / URunner Repro

## When to use

- Turn a CI regression into a local checked or URunner repro.
- Choose the fastest harness for compiler-sensitive failures.
- Extract the exact `ark` or `ark_aot` command hidden behind a runner.
- Write or trim a checked test around JIT, AOT, or PGO behavior.

## Current sources of truth

- `../../tests/AGENTS.md` for the current harness map and reproduction discipline.
- `../../tests/checked/README.md` for checker DSL and `RUN_*` command behavior.
- `../../tests/tests-u-runner/readme.md` for URunner v1 options and caveats.
- `../../tests/tests-u-runner-2/README.md` for URunner v2 usage and environment requirements.
- `../../docs/flaky_debugging.md` for the current repro-minimization workflow.
- `../../runtime/options.yaml` for runtime-side flags that appear in extracted commands.
- `../../compiler/tools/paoc/paoc.yaml` for current `ark_aot` flag names.
- `../../compiler/compiler.yaml` for compiler dump and log flags used during minimization.

## Workflow

1. Choose the lightest correct harness.

- Use checked first for IR, codegen, disasm, event, or pass-sensitive failures.
- Use URunner v1 or v2 for CI parity, larger workflows, or suite-driven failures.
- Once you have a reproducer, minimize it to raw `ark` or `ark_aot` commands before deeper debugging.

2. For checked repros, start from the exact invocation.

- Prefer `ninja -v <target>.checked` to capture the current `checker.rb` command line.
- Keep the first checked version tiny: one entry point, one mode, one method if possible.
- Use current paoc flags inside checked options as well.

3. For URunner repros, narrow before instrumenting.

- URunner v2 requires initialized environment variables such as `ARKCOMPILER_RUNTIME_CORE_PATH`, `ARKCOMPILER_ETS_FRONTEND_PATH`, `PANDA_BUILD`, and `WORK_DIR`.
- URunner v1 `--handle-timeout` still requires rerunning under `sudo`.
- Keep `--verbose all`, `--filter`, `--test-file`, and `--test-list` close to the first repro.

## Checked DSL crib sheet

- `RUN`: execute the program under `ark`. Use `force_jit: true` only when you really want `--compiler-enable-jit=true --compiler-hotness-threshold=0 --no-async-jit=true`.
- `RUN_PAOC`: compile with `ark_aot` before the subsequent `RUN`.
- `RUN_AOT`: exercise AOT paths through checked infrastructure.
- `RUN_PGO_PROF`: collect profile data for a later PGO-enabled AOT run.
- `RUN_PGO_PAOC`: compile with collected profile data.
- `METHOD`: select one method for IR or disasm checks.
- `PASS_AFTER`: bind IR checks to one compiler pass.
- `INST`: assert IR content in the selected method and pass.
- `ASM_METHOD`: narrow disassembly checks to one method.
- `EVENT`: assert compiler or runtime event presence.

## Common pitfalls

- Do not start with a whole suite when a checked repro would isolate the issue faster.
- Do not keep debugging inside URunner output after the exact `ark` or `ark_aot` command is available.
- Do not forget that `force_jit: true` expands to `--compiler-enable-jit=true --compiler-hotness-threshold=0 --no-async-jit=true`.
- Do not use historical paoc option names inside checked `options:` strings.
- Do not assume URunner2 is broken until its environment variables are confirmed.

## Escalate / handoff

- If the runner repro is stable and now needs IR or disassembly analysis, continue in `../ark-ir-disasm-triage/SKILL.md`.
- If the next step is a raw interpreter/JIT/AOT command matrix, continue in `../ark-jit-aot-repro/SKILL.md`.
- If the issue is really profile-guided AOT behavior, continue in `../ark-aot-pgo/SKILL.md`.
