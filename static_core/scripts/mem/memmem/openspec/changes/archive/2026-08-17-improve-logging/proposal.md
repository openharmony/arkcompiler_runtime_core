## Why

The benchmark runner currently mixes user-facing status, warnings, fatal errors, and device hilog collection under ambiguous `logs` naming and direct `print(...)` calls. Adding explicit memmem logging controls will make runner progress observable when needed, keep default output quiet, and separate application logs from collected OpenHarmony `hilog` artifacts.

## What Changes

- Add memmem application logging for runner-level benchmark execution.
- Add log levels `info`, `warn`, and `err`, defaulting to `err`; emitted prefixes use full words `info:`, `warning:`, and `error:`.
- Add CLI options `--memmem-log-level {info,warn,err}` and `--memmem-log-file PATH`, with empty log file meaning stdout/stderr routing and non-empty log file meaning all memmem logs are overwritten into that file.
- **BREAKING** Rename CLI hilog controls from `--logs` / `--no-logs` to `--hilog` / `--no-hilog` with no compatibility aliases.
- **BREAKING** Rename the `lib.run()` keyword option from `logs` to `hilog`, with no compatibility alias.
- **BREAKING** Rename run-wide hilog output directories from `logs/hilog.log` to `hilog/hilog.log` locally and remotely.
- Keep report and plot generation free of direct memmem logger invocations for this scope.
- Keep `record.py` logging behavior unchanged.

## Capabilities

### New Capabilities
- `memmem-logging`: Memmem application logging levels, destinations, and message semantics for runner execution and result processing.

### Modified Capabilities
- `benchmark-flow`: CLI and runtime option requirements change from `logs` controls to `hilog` controls and add memmem logging options.
- `programmatic-interface`: `lib.run()` runtime options change from `logs` to `hilog`, and memmem logging is configured separately through public logger helpers.
- `log-streaming`: Run-wide device hilog collection is controlled by `hilog` options and uses `hilog/hilog.log` paths.
- `result-evidence`: Output layout and pending artifact paths change from `logs/hilog.log` to `hilog/hilog.log`.

## Impact

- Affected code: `run.py`, `lib.py`, `src/runner.py`, `src/report.py`, `src/plot.py`, `src/result.py`, and tests under `test/`.
- Public CLI/API breaking changes: remove `--logs` / `--no-logs`, remove `logs=` from `lib.run()`, and rename hilog artifact directories.
- No new third-party dependencies are required.
- Existing tests that assert stderr warnings, CLI options, `logs=` API usage, or `logs/hilog.log` paths must be updated.
