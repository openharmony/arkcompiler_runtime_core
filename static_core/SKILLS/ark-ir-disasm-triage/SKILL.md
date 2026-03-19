---
name: ark-ir-disasm-triage
description: Triage Ark static_core compiler issues with IR dumps, disassembly, `ark_aot --paoc-mode=jit|osr|aot|llvm`, `ark_aotdump`, compiler logs, and method narrowing through `--compiler-regex`. Use when comparing pass-local behavior, verifying code generation, or deciding whether a failure is compiler-local or belongs to the runtime boundary.
---

# Ark IR / Disasm Triage

## When to use

- Compare IR before and after one pass for a single method.
- Inspect generated native code or disassembly for JIT, OSR, AOT, or LLVM AOT shapes.
- Decide whether a miscompile is inside the compiler pipeline or after runtime installation.
- Localize a bug after `LoopUnswitch`, inlining, lowering, register allocation, or code generation.

## Current sources of truth

- `../../compiler/compiler.yaml` for dump, disassembly, regex, and log flags.
- `../../compiler/tools/paoc/paoc.yaml` for `ark_aot` mode and output flags.
- `../../compiler/AGENTS.md` for compiler workflow, modes, and tool ownership.
- `../../runtime/AGENTS.md` for runtime/compiler boundary ownership and post-installation clues.
- `../../docs/flaky_debugging.md` for the current crash-localization workflow.
- `../../compiler/docs/paoc.md` for current offline `ark_aot` modes.
- `../../docs/aot.md` for `.an` inspection and runtime-visible AOT behavior.

## Workflow

1. Isolate one method.

- Add `--compiler-regex='Class::method'` as early as possible.
- If the failure is mode-specific, keep only one mode active.
- For offline validation, prefer `ark_aot --paoc-mode=jit|osr|aot|llvm`.

2. Dump IR.

- Start with `--compiler-dump`.
- Add `--compiler-dump:final` when full pass-by-pass output is too noisy.
- Add `--compiler-dump:bytecode` when bytecode-to-IR mapping matters.
- Add `--compiler-dump:life-intervals` when register allocation looks suspicious.

3. Dump disassembly.

- Use `--compiler-disasm-dump:single-file` for one consolidated file.
- Use `ark_aotdump` for already-produced `.an` output.

4. Narrow by pass or logger.

- Add `--compiler-log=<component>` only for the passes you are chasing.
- Compare one before/after delta, not the whole pipeline at once.
- Remember that OSR mode is a stricter pipeline and currently skips inlining.

5. Decide ownership.

- If the bad shape is already present in IR or disasm, stay in compiler-owned code.
- If the symptom appears only after installation, frame transition, or AOT loading, switch to runtime-boundary debugging.

### Typical offline JIT-shape dry-run command

```bash
${BUILD}/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --paoc-mode=jit \
  --compiler-regex='App::hotMethod' \
  --compiler-dump \
  --compiler-dump:bytecode \
  --compiler-disasm-dump:single-file
```

Switch to `--paoc-mode=aot` when you need normal AOT backend output, or `--paoc-mode=llvm` when LLVM-backed code
generation is the actual subject of the investigation.

### Inspect compiled AOT output

```bash
${BUILD}/bin/ark_aotdump --show-code disasm app.an
```

## Common pitfalls

- Do not inspect many methods at once when one regex would do.
- Do not stay in compiler dumps if the failure only appears after code installation.
- Do not use historical paoc option names; keep `--paoc-panda-files` and `--paoc-mode`.
- Do not assume OSR output must match JIT output; the pipeline is intentionally narrower.

## Escalate / handoff

- If you still need a stable raw repro command matrix, continue in `../ark-jit-aot-repro/SKILL.md`.
- If the failure is clearly in bridges, stack walking, AOT loading, or post-installation behavior, continue in `../../runtime/AGENTS.md`.
- If the issue appears only under checked or URunner harnesses, continue in `../ark-checked-urunner-repro/SKILL.md`.
