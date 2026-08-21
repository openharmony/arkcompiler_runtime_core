## 1. Logger Foundation

- [x] 1.1 Add an internal memmem logger module with `info`, `warn`, and `err` levels, full-word prefixes, level filtering, and validation.
- [x] 1.2 Implement stdout/stderr routing for empty `memmem_log_file` and overwrite-only file routing for non-empty `memmem_log_file`.
- [x] 1.3 Add logger tests covering filtering, prefixes, stdout/stderr routing, file overwrite routing, invalid levels, and missing file parent failure.

## 2. CLI and Programmatic API

- [x] 2.1 Update `run.py` parser to add `--memmem-log-level`, `--memmem-log-file`, `--hilog`, and `--no-hilog`, and remove `--logs` / `--no-logs`.
- [x] 2.2 Update `run.py` to log CLI phase transitions and fatal errors through memmem logging.
- [x] 2.3 Update `lib.run()` to replace `logs` with `hilog`, expose public logger helpers from `lib.py`, and use the singleton logger through execution and reports.
- [x] 2.4 Update CLI/API tests for new defaults, rejected removed options, invalid log levels, fatal error output, and removed `logs=` keyword behavior.

## 3. Hilog Rename

- [x] 3.1 Rename benchmark runtime option fields and call sites from generic `logs` naming to `hilog` where they control device hilog collection.
- [x] 3.2 Rename result path helpers and output layout from `logs/hilog.log` to `hilog/hilog.log` locally and remotely.
- [x] 3.3 Update hilog-related runner, result, fake-device, and output layout tests for the new option and directory names.

## 4. Runner Logging

- [x] 4.1 Use the `src.log` singleton logger from `run_benchmark()` and runner helper functions without adding logger parameters to function signatures.
- [x] 4.2 Add `info` logs for required runner transitions: benchmark preparation, flow verification, flow execution, AppFlow execution, finalization, child-process shutdown, metadata writing, artifact receiving, and remote cleanup.
- [x] 4.3 Add `info` logs for every command execution with AppFlow label and command action, without logging `text` or `input_text` payload contents.
- [x] 4.4 Add runner tests verifying transition logs, command execution logs, and text payload redaction at `info` level.

## 5. Report and Plot Scope

- [x] 5.1 Keep report and plot generation free of direct memmem logger calls.
- [x] 5.2 Keep current runner-level transition logging granularity as the logging scope for this change.
- [x] 5.3 Update report and plot tests for silent skip behavior without warning log assertions.

## 6. Documentation and Verification

- [x] 6.1 Update README CLI, programmatic API, and output layout documentation for memmem logging options, `hilog` naming, and `hilog/hilog.log` paths.
- [x] 6.2 Run focused unit tests for logger, CLI, API, runner, report, plot, and result path changes.
- [x] 6.3 Run `make tests_full` and inspect diffs after autopep8 formatting.
