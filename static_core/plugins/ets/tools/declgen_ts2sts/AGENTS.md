# AGENTS.md — declgen_ts2sts

> Project guide for AI coding agents. Human developers should read [README.md](README.md) first.

## 1. Project Overview

`declgen_ts2sts` is a tool that converts TypeScript source code into ArkTS `.d.ets` declaration files.

It:

- generates `.d.ets` based on cookbook rules,
- autofixes a subset of incompatible TS patterns,
- emits JSON-formatted diagnostics for code that cannot be autofixed (not implemented yet).

Entry points: [src/main.ts](src/main.ts) → [src/cli/DeclgenCLI.ts](src/cli/DeclgenCLI.ts) → [src/Declgen.ts](src/Declgen.ts) → [src/Pipeline.ts](src/Pipeline.ts).

## 2. Directory Layout

```
src/
  main.ts                 # Executable entry (the `bin` in package.json)
  index.ts                # Library entry
  Declgen.ts              # Top-level orchestration
  Pipeline.ts             # Generic Stage pipeline
  cli/                    # Command-line parsing (commander)
  compiler/               # TS compiler wrapper, VirtualFileHost
  logger/                 # Logger abstraction + Console/Silent implementations
  stages/                 # Individual transformation stages (core logic)
    ast-fix-stage/
    conversion-stage/
    declaration-conversion-stage/
    interop-tags-stage/
    interop-types-conversion-stage/
    recheck-stage/
    semantic-fix-stage/
  utils/

test/
  framework/              # In-house cookbook test framework (Runner/Executor/Worker)
  cookbook_tests/         # Test cases (ts_source / dts_declarations / package_source ...)
  unit/                   # mocha unit tests

build/                    # tsc output; do not edit by hand
script/install_tsc.sh     # Installs the customized ohos-typescript
```

When adding features, prefer modifying an existing `stage` over creating a new one. The Stage interface lives in [src/stages/Stage.ts](src/stages/Stage.ts); wiring happens in [src/Pipeline.ts](src/Pipeline.ts).

## 3. Build & Run

Requirements: Node.js (use the version agreed on by the team) and npm. The `postinstall` hook runs `script/install_tsc.sh` automatically to install `ohos-typescript@4.9.5-r9`.

```bash
npm i                 # Install dependencies (includes the customized tsc)
npm run build         # clean + tsc → build/
npm run declgen -- -f path/to/file.ts -o out/
```

## 4. Testing

```bash
npm test                       # Compile + run the full cookbook suite (parallel)
npm run test:unit              # mocha unit tests only (build/test/unit/**)
npm run test:cookbook-one -- test <name> \
    --test-dir ./test/cookbook_tests/ \
    --test-out-dir ./build/ \
    -t ts_source                # Run a single case; handy for debugging
```

Conventions:

- After changing stage logic, you must run `npm test`, then `git diff` the affected cookbook cases to make sure `dets_output/` has no unintended regressions.
- New rules must come with matching inputs / expected outputs under `test/cookbook_tests/`.

## 5. Code Style

```bash
npm run eslint-check          # Lint
npm run fix                   # prettier + eslint --fix
```

- Required: Apache-2.0 file header (see the top 14 lines of any existing `.ts` file). Use the current year.
- TypeScript strict mode; avoid `any`. For AST-related utility code, annotate node types explicitly.
- Do not import `typescript` directly — always import from the project's existing wrapper so that `ohos-typescript` is used.
- Use the logger from [src/logger](src/logger/index.ts); do not use `console.log`.

## 6. Key Constraints / Common Pitfalls

- **Do not hand-edit build artifacts in `build/`** — they are wiped by `npm run clean`.
- **Use the customized tsc**: a globally installed `tsc` may behave differently; always use `npm run tsc` when debugging.
- **Stages form a pure functional pipeline**: pass data through `StageContext<T>`; do not introduce global state between stages.
- **Diagnostics** are centralized in [src/stages/TransformationDiagnosticMessages.ts](src/stages/TransformationDiagnosticMessages.ts). When adding error codes, append them here and keep numbering contiguous.

## 7. Pre-commit Checklist

- [ ] `npm run build` passes
- [ ] `npm test` passes (or the PR explains which cases are intentionally updated)
- [ ] `npm run prettier-fix` applied
- [ ] New files include the Apache-2.0 header; modified files have the current year in the trailing copyright line.
- [ ] Relevant test cases / docs (README) updated

## 8. Extra Tips for Agents

- When exploring the code, start with [src/Pipeline.ts](src/Pipeline.ts) and [src/Declgen.ts](src/Declgen.ts) to understand the overall data flow.
- When fixing a bug, reproduce first: use `npm run test:cookbook-one` on the smallest case, then narrow down to the responsible stage.
- Do not leave temporary debug `console.log` calls in stage code.
- Do not claim "no regressions" without actually running the tests.
- The number of function lines should not exceed 50 lines.
- The depth of function should not exceed 5.
