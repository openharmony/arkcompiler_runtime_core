# Flaky Debugging Workflows

This note describes the current flaky-debugging workflow for `static_core`.

## First Principles

- Reproduce on the same build type, sanitizer configuration, architecture, and runner workflow that failed in CI.
- Ensure `runtime_core` and `ets_frontend` revisions match the failing setup before chasing a low-level hypothesis.
- CI infrastructure can be noisy. If the failure looks environmental, rerun once before assuming a product regression.
- Pass `--interpreter-type=cpp|irtoc|llvm` explicitly for local repros. The runtime default is declared as `llvm`, but
  it may downgrade at runtime depending on build flags and configuration.

## Reproducing by Test Layer

### Checked tests

Checked tests remain the fastest path for compiler or IR-sensitive repros.

- Use `ninja -v <target>.checked` to expose the exact `checker.rb` invocation.
- `checker.rb` recreates the per-test directory, so stale artifacts should not be treated as expected behavior.
- Keep `--compiler-regex`, `--compiler-dump`, and `--compiler-disasm-dump:single-file` close to the repro from the
  beginning.

### URunner v1

Current sources of truth:

- `tests/tests-u-runner/readme.md`
- `tests/tests-u-runner/runner.sh`

Useful current options:

- `--verbose all`
- `--force-generate`
- `--generate-only`
- `--aot`
- `--jit`
- `--interpreter-type`
- `--filter`
- `--test-file`
- `--test-list`

Important current caveat:

- `--handle-timeout` requires rerunning under `sudo`. The code path that collects thread dumps checks this explicitly.

### URunner v2

Current sources of truth:

- `tests/tests-u-runner-2/README.md`
- `tests/tests-u-runner-2/runner.sh`

Useful current options:

- `--verbose all`
- `--force-generate`
- `--generate-only`
- `--filter`
- `--test-file`
- `--test-list`
- `--enable-es2panda false`
- `--enable-verifier false`

Important current caveats:

- URunner2 requires environment initialization, either through `.urunner.env` or explicit environment variables such as
  `ARKCOMPILER_RUNTIME_CORE_PATH`, `ARKCOMPILER_ETS_FRONTEND_PATH`, `PANDA_BUILD`, and `WORK_DIR`.
- Failure reports already include `To reproduce with URunner run` plus the required environment variables.

## Minimizing to Exact Tool Commands

Runner-level repro is usually only the first step. For flaky crashes or miscompiles, minimize further:

1. rerun a single test with `--test-file`, `--filter`, and `--verbose all`
2. capture the exact `es2panda`, verifier, `ark_aot`, and `ark` commands
3. rerun the smallest command that still reproduces
4. only then add loops, load, debugger, or compiler dumps

Current tree behavior helps here:

- URunner v1 stores reproduction steps and prints `To reproduce with URunner run`.
- URunner v2 appends the exact per-step command lines to the reproduction output, not just the top-level runner
  command.

If you need high-load repro, the repository does not provide a dedicated `GNU parallel` wrapper. Extract the exact
`ark` or `ark_aot` command first, then use any external load tool you trust.

## Compiler and AOT Crash Localization

Once you have a direct `ark` or `ark_aot` command, current repo-supported tools are:

- `--compiler-regex='Class::method'`
- `--compiler-dump`
- `--compiler-dump:final`
- `--compiler-disasm-dump:single-file`
- `--compiler-log=...`
- `--compiler-enable-events`
- `ark_aotdump`

For AOT linkage or entrypoint issues:

- use current logger options such as `--log-debug=AOT`
- `runtime/class_linker.cpp` still contains `MaybeLinkMethodToAotCode`, which is a useful breakpoint when debugging
  AOT entry installation

Typical workflow:

1. isolate one method with `--compiler-regex`
2. dump IR and disassembly
3. if needed, compile only that method with `ark_aot --paoc-panda-files=... --paoc-mode=aot`
4. map the failing native offset back to `disasm.txt`
5. locate the corresponding IR instruction and the pass or codegen site that produced it

## GC-Sensitive Flakes

Use the same repro discipline for GC bugs, then add GC-specific visibility:

- `--log-level=debug --log-components=gc:mm-obj-events`
- `--log-detailed-gc-info-enabled=true`
- `--print-gc-statistics=true`
- `--print-memory-statistics=true`

For ETS-specific experiments, the current stdlib exposes:

- `GC.startGC(...)`
- `GC.postponeGCStart()`
- `GC.postponeGCEnd()`

These are useful for targeted experiments, but benchmark and flaky repro scripts should keep GC-related changes
explicit and documented.

## Related Docs

- `../tests/AGENTS.md`
- `../compiler/AGENTS.md`
- `../runtime/AGENTS.md`
- `../tests/tests-u-runner/readme.md`
- `../tests/tests-u-runner-2/README.md`
- `../tests/checked/README.md`
