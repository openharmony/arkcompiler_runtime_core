## Why

Benchmark runs are easier to compare when device preparation is explicit and repeatable, runtime logs are preserved as evidence, and breakdown output prioritizes the largest memory contributors. The framework should optionally reboot before running, always prevent screen timeout during a benchmark, collect per-AppFlow hilog logs by default, and make breakdown CSVs easier to inspect.

## What Changes

- Add `BenchmarkOptions` in `src.options` containing `flow_path`, `out_dir`, `reboot`, and `logs`.
- **BREAKING** for internal callers: introduce `run.py::run(device, options)` as the full orchestration entry point and change lower-level benchmark evidence collection to `run_benchmark(options, store, device)` with no default options argument.
- Add CLI flags `--reboot`/`--no-reboot` with default `reboot=False`.
- Add CLI flags `--logs`/`--no-logs` with default `logs=True`.
- Add HDC timeout support with `TIMEOUT_ERR_CODE = 127` and timeout result `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")`; `timeout=-1` means no timeout.
- Add device operations for reboot, HDC wait, boot-complete polling, screen-timeout disabling, hilog configuration, hilog clearing, and hilog dumping.
- Always run `power-shell timeout -o 60000000` before AppFlows, independent of CLI options.
- When logs are enabled, configure hilog after environment preparation, clear hilog before each AppFlow, dump `hilog -x` after each AppFlow, and receive logs into `logs/<app_label>.log` under the benchmark output directory.
- Preserve original AppFlow errors when log collection also fails; use a combined error if straightforward.
- Sort breakdown CSV rows by `Size_total_for_tag` descending, with tag ascending as deterministic tie-breaker.
- Update shared test fakes to support the new options, device operations, log artifacts, and HDC timeout simulation.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: CLI options, benchmark options object, optional reboot environment preparation, always-on screen-timeout disabling, and log hook integration around AppFlows.
- `device-hdc`: HDC timeout support and device operations for reboot, boot readiness, power timeout, and hilog control/dump.
- `result-evidence`: Per-AppFlow log artifact layout and receive behavior; breakdown CSV sort order.
- `testing-support`: Fake HDC timeout simulation and fake device support for reboot/log operations.

## Impact

- Affected code: `run.py`, `src/options.py`, `src/hdc.py`, `src/device.py`, `src/runner.py`, `src/result.py`, `src/report.py`, and tests under `test/`.
- New output artifacts: `logs/<app_label>.log` when `--logs` is enabled.
- Internal API change: full benchmark callers use `run.py::run(device, options)`; lower-level evidence collection callers of `run_benchmark()` must pass `BenchmarkOptions`, `ResultStore`, and `Device`.
- No new external dependencies.
