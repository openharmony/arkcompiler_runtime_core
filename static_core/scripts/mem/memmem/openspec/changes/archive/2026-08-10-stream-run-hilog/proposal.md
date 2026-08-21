## Why

Current hilog collection clears the hilogd buffer before each AppFlow and dumps `hilog -x` after that AppFlow. OpenHarmony sources show `hilog -x` is a non-blocking read of the current hilogd buffer, whose default per-type size is 256 KiB, so high-volume or early runtime events can be overwritten before collection.

When reboot is enabled, the current model also starts collection too late to preserve full AppFlow runtime coverage from before the first AppFlow launch.

## What Changes

- **BREAKING**: `--logs` changes from per-AppFlow buffered dumps to one run-wide hilog artifact.
- **BREAKING**: reboot default changes from `--reboot` to `--no-reboot`; users must opt into rebooting explicitly.
- Replace per-AppFlow `hilog -r` and `hilog -x > <remote>` collection with one device-local streaming command `hilog > <remote_run_dir>/logs/hilog.log`.
- Add a blocking device API for hilog streaming and run it from the runner in a separate multiprocessing child.
- Start streaming after health verification and execution context preparation succeed, before AppFlow launch.
- Stop streaming in benchmark cleanup/finalization on success and failure, aggregating stop/receive errors with the existing error-combination mechanism.
- Keep the existing pending artifact receive mechanism for `logs/hilog.log`.
- Do not add `log_metadata.json`; the single artifact path is fixed at `logs/hilog.log`.
- Keep `--logs` / `--no-logs` and `--reboot` / `--no-reboot` explicit controls, with `--logs` and `--no-reboot` as defaults.

## Capabilities

### New Capabilities

- `log-streaming`: run-wide device-local hilog stream lifecycle behavior.

### Modified Capabilities

- `benchmark-flow`: log collection timing and lifecycle change from per-AppFlow buffered dumps to run-wide streaming.
- `device-hdc`: device API changes for blocking device-local hilog streaming.
- `result-evidence`: log output layout changes from per-AppFlow log artifacts to one fixed run-wide log artifact.
- `testing-support`: fake device/process support for stream lifecycle and failure cases.

## Impact

- `run.py`: CLI help text for `--logs` / `--no-logs`.
- `src/options.py`: `BenchmarkOptions.logs` semantics only; field remains unchanged.
- `src/device.py`: replace `clear_hilog()` / `dump_hilog()` usage with `hilog(remote_path)`.
- `src/runner.py`: create remote logs dir, start/stop child processes at run scope, and keep one pending log artifact.
- `src/commands.py`: `ExecutionContext` owns the flow, child processes, and a flat list of pending artifacts.
- `src/result.py`: add fixed run-wide log path helpers while keeping remote/local logs directory helpers.
- `test/mock/device.py`: add blocking hilog fake support and stream lifecycle failure knobs.
- Tests and README/specs referencing per-AppFlow logs need updates.
