## Context

The benchmark path currently has two unrelated concepts named as logs: memmem runner/user messages printed directly by Python, and run-wide OpenHarmony `hilog` artifacts collected from the device. `run.py` emits status and fatal messages with direct `print(...)`; report and plot code emit warnings directly to stderr; `lib.run()` exposes a `logs` keyword that actually controls device hilog collection. The current output layout stores the device hilog artifact at `logs/hilog.log`.

This change separates memmem application logging from device hilog collection. Memmem logging applies to runner-level benchmark execution paths exposed by `run.py` and `lib.run()`. Report and plot code do not invoke the memmem logger directly. `record.py` remains unchanged.

## Goals / Non-Goals

**Goals:**
- Add explicit memmem application logging with levels `info`, `warn`, and `err`.
- Keep default runner output quiet except fatal errors by defaulting to `err`.
- Add CLI and programmatic controls for log level and optional log file destination.
- Log major runner transitions at `info`, including benchmark preparation, flow verification, flow execution, app-flow execution, finalization, metadata writing, artifact receiving, and remote cleanup.
- Log every command execution at `info` without exposing `text` or `input_text` contents.
- Keep report and plot generation free of direct memmem logger invocations for the current scope.
- Rename device hilog controls and artifact directories from generic `logs` naming to explicit `hilog` naming.
- Apply breaking CLI/API/path renames immediately without compatibility aliases.

**Non-Goals:**
- Do not change `record.py` logging, warning, prompt, or error behavior.
- Do not add a third-party logging dependency.
- Do not add a no-output log level beyond `info`, `warn`, and `err`.
- Do not change the device-side `hilog` command semantics beyond renamed controls and paths.
- Do not log fatal errors from lower-level helper functions before re-raising; fatal logging stays at the CLI boundary.

## Decisions

### Add a small internal memmem logger

Create an internal logger abstraction, likely in `src/log.py`, rather than adopting Python's standard logging configuration globally. The project needs deterministic CLI-oriented messages, simple level filtering, and explicit stdout/stderr/file routing. A small local abstraction avoids global logging state and keeps tests straightforward.

The logger accepts levels `info`, `warn`, and `err`. Filtering is ordered so `err` emits only errors, `warn` emits warnings and errors, and `info` emits info, warnings, and errors. Emitted prefixes use full words: `info:`, `warning:`, and `error:`.

### Route by destination configuration

When `memmem_log_file` is empty, informational and warning logs go to stdout and error logs go to stderr. When `memmem_log_file` is non-empty, all emitted memmem logs go only to that file. The log file is opened in overwrite mode and its parent directory must already exist; missing parents fail normally instead of being created implicitly.

### Use a module-level singleton logger

Use `src.log` as the single owner of memmem logging state. `lib.py` exposes public `configure_logger`, `get_logger`, and `reset_logger` helpers. `run.py` configures the logger through `lib.configure_logger()` before CLI flow validation, and lower layers obtain it from `src.log.get_logger()` instead of accepting logger parameters or storing logger references in execution context.

`lib.run()` does not accept memmem logging arguments and does not configure logging. Programmatic callers configure logging separately before invoking `lib.run()`. This keeps runtime execution options separate from logger setup and avoids polluting runner, report, plot, and command function signatures with logging arguments.

### Keep fatal logging at the CLI boundary

`run.py` catches top-level exceptions and prints `error: memmem error: ...` before returning `1`. `lib.run()` continues to report failures by raising exceptions and does not catch/log/rethrow fatal errors. Lower layers raise exceptions without also logging errors, avoiding duplicate messages and preserving existing combined-error behavior during cleanup.

### Rename device hilog controls immediately

Rename CLI controls from `--logs` / `--no-logs` to `--hilog` / `--no-hilog` with no aliases. Rename `lib.run(logs=...)` to `lib.run(hilog=...)` with no compatibility parameter. Rename internal runtime option fields from `logs` to `hilog` where practical.

The output layout changes from `logs/hilog.log` to `hilog/hilog.log` locally and remotely. Result path helpers should be renamed from `local_logs_dir()` / `remote_logs_dir()` to `local_hilog_dir()` / `remote_hilog_dir()` while retaining `local_hilog_path()`, `remote_hilog_path()`, and `hilog_relative_parts()` concepts.

### Info logging granularity

`info` logs cover every major transition and every command execution. The minimum transition set is:

- start benchmark preparation
- start flow verification
- start flow execution
- start app flow
- execute command
- start post-flow cleanup/finalization
- start child-process shutdown
- start metadata writing
- start artifact receiving
- start remote cleanup

Command logs include app label and action. They must not include sensitive text payload values for `text` or `input_text`; a safe default is to omit all command payloads from command execution logs unless a future requirement explicitly permits selected non-sensitive payloads.

## Risks / Trade-offs

- **Breaking scripts that use `--logs`, `--no-logs`, or `logs=`** → Mitigation: update README, tests, and public examples in the same change; fail fast with argparse/API errors rather than silently preserving old names.
- **CLI fatal errors bypass selected memmem log file** → Mitigation: keep fatal boundary simple and visible on stderr while runner logs obey configured routing.
- **Info logs can become noisy for long flows** → Mitigation: `info` is opt-in and default level remains `err`.
- **Command logs may leak typed text if payloads are included later** → Mitigation: initial design omits command payloads from execution logs and tests should cover `text` and `input_text` redaction.
- **Logger threading/resource lifecycle complexity for files** → Mitigation: keep logging synchronous, line-oriented, and scoped to the process run; overwrite log files and close/flush deterministically where the logger owns the file stream.
- **Spec and documentation churn from hilog path rename** → Mitigation: update affected specs, README output layout, and tests together.

## Migration Plan

1. Add the memmem logger abstraction and tests for filtering, prefixes, stream routing, file overwrite behavior, and invalid levels.
2. Rename hilog options and path helpers in code and tests.
3. Expose logger configuration through `lib.py` and read the singleton logger from `runner.py`.
4. Add info logs for required runner transitions and command execution.
6. Update README examples and output layout.
7. Run `make tests_full` and inspect formatting changes from `autopep8`.

Rollback is straightforward before release by reverting this change. After release, rollback would require either restoring old `logs` controls or documenting another breaking rename.

## Open Questions

None.
