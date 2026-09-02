## Context

The framework already separates CLI wiring from runner execution through `run.py`, `Device`, and `run_benchmark()`. The next step is to make benchmark environment choices explicit while keeping CLI option handling outside the main AppFlow loop. The change adds a single options object, optional reboot preparation, always-on screen-timeout prevention, per-AppFlow hilog evidence, and size-prioritized breakdown rows.

## Goals / Non-Goals

**Goals:**
- Add `BenchmarkOptions` in `src.options` with `flow_path`, `out_dir`, `reboot`, and `logs`.
- Keep benchmark orchestration explicit by using `run.py::run(device, options)` for full runs and `run_benchmark(options, store, device)` for lower-level evidence collection, with no default options argument.
- Support `--reboot`/`--no-reboot` with default `False`.
- Support `--logs`/`--no-logs` with default `True`.
- Add HDC timeout handling where `timeout=-1` means no timeout and timeout expiry returns `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")`.
- Prepare the device before AppFlows by optionally rebooting, waiting for boot completion, always disabling screen timeout, and optionally configuring hilog.
- Capture all-window hilog logs for each AppFlow and receive them into the output directory.
- Sort breakdown CSV rows by `Size_total_for_tag` descending.

**Non-Goals:**
- Unlock devices or handle boot-time dialogs.
- Initialize the desktop or launcher; `aa start` brings benchmark applications to the foreground.
- Filter logs by PID in the initial implementation.
- Add persistent hilog configuration.
- Add new external dependencies.

## Decisions

### Options are centralized in `src.options`

Introduce:

```python
@dataclasses.dataclass(frozen=True)
class BenchmarkOptions:
    flow_path: pathlib.Path
    out_dir: pathlib.Path
    reboot: bool = False
    logs: bool = True
```

`run.py` parses CLI flags and constructs `BenchmarkOptions`. CLI boolean control pairs are parsed as mutually exclusive groups so contradictory pairs such as `--logs --no-logs` and `--reboot --no-reboot` fail during argument validation. `run.py::run(device, options)` is the full orchestration entry point: it creates the `ResultStore`, calls `run_benchmark(options, store, device)`, and then generates reports. `run_benchmark()` is the lower-level evidence collection routine and requires explicit `BenchmarkOptions`, `ResultStore`, and device-compatible objects with no default options argument. This keeps execution configuration explicit, makes result storage injectable, and keeps report generation outside device/artifact collection.

The CLI currently creates a timestamped output directory named `memmem-out-YYYYMMDD_HHMMSS_microseconds`; no `--out` override is exposed in this change because host path handling can be confusing in WSL/HDC interop scenarios.

Alternative considered: keep `flow_path` and `out_dir` as separate runner arguments and pass only boolean options separately. A single options object keeps CLI-provided benchmark configuration together and avoids signature growth.

### Environment preparation is separate from the AppFlow loop

Add helper-level structure:

```text
run.py::run(device, options)
  create store with matching local and remote output directory names
  run_benchmark(options, store, device)
    parse flow
    initialize local output (flow copy, snapshots/, breakdowns/, and logs/ when enabled)
    prepare_environment(device, options)
    read screen bounds
    initialize remote output (run root, snapshots/, and logs/ when enabled)
    execute_app_flows(...)
    write metadata
    receive artifacts
    cleanup
  report
```

`prepare_environment()` handles:

```text
if options.reboot:
  target boot
  hdc wait
  poll bootevent.boot.completed
always:
  power-shell timeout -o 60000000
if options.logs:
  hilog -Q pidoff
  hilog -p off
```

This keeps CLI option effects out of the main AppFlow loop.

### HDC timeout support returns an HdcResult error

Add `TIMEOUT_ERR_CODE = 127`. `Hdc.run(..., timeout=-1)` keeps current no-timeout behavior. Non-negative timeouts are passed to `subprocess.run`; `subprocess.TimeoutExpired` returns `HdcResult(TIMEOUT_ERR_CODE, "", f"memmem: TIMEOUT {cmd}")`.

`Hdc.shell()` forwards the same timeout parameter to `run()`.

Alternative considered: raise on timeout. Returning `HdcResult` preserves the current wrapper policy that callers decide whether a command is an error.

### Boot readiness is transport plus boot parameter

`Device.reboot()` uses `hdc target boot`. `Device.wait_available(timeout_seconds)` uses `hdc wait` with timeout. `Device.wait_boot_completed(timeout_seconds, poll_interval_seconds=1)` polls `hdc shell param get bootevent.boot.completed` until stdout is `true` or timeout expires.

No desktop initialization occurs because app launch through `aa start` brings the tested app to the foreground.

### Logs are all-window AppFlow artifacts

When logs are enabled:

```text
configure once after environment prep:
  hilog -Q pidoff
  hilog -p off

for each AppFlow:
  before launch:
    hilog -r
  after flow body, even on failure:
    hilog -x > remote logs/<app_label>.log
```

The implementation should include the requested code comment immediately above `dump_hilog()`:

```python
# If app-only logs are needed later, pass -P <pid> to hilog.
```

All-window logs are chosen because clearing the hilog buffer before each AppFlow makes the artifact correspond to that AppFlow's time window without risking missed child-process or service logs.

### Original AppFlow failures are not masked by log failures

If an AppFlow succeeds and log collection fails, the benchmark fails with the log collection error. If an AppFlow fails and log collection also fails, prefer a combined error that preserves the original AppFlow failure as the cause; if combining becomes awkward, preserve the original AppFlow error rather than masking it.

### Logs are received before cleanup and report generation

Snapshot and log artifact paths use shared `RelativeParts = list[str]`. Log artifacts are tracked in `ExecutionContext.logs` as `["<app_label>.log"]`. Snapshot and log artifacts are grouped into one pending artifact receive step before remote cleanup. The receiver attempts every pending artifact group, so logs are still attempted if snapshot receive fails and snapshots are still attempted if log receive fails. If AppFlow execution fails after staging any artifacts, the benchmark still writes available metadata, attempts to receive all staged artifact groups, and then performs cleanup.

The remote run directory mirrors the local output directory name under `/data/local/tmp`, which makes host/device artifact comparison easier while relying on the timestamped local output name for uniqueness.

Remote cleanup is attempted after the remote run directory is created, including when AppFlow execution, metadata writing, or artifact receive fails. Cleanup failure is combined with the original error when needed. Report generation runs after `run_benchmark()` succeeds.

No log metadata file is required for the first version.

### Breakdown rows sort by size descending

`write_breakdown_csv()` sorts by `profile.size_kb` descending and tag ascending as tie-breaker. This makes the largest memory contributors appear first while preserving deterministic output for equal sizes.

## Risks / Trade-offs

- **Risk:** Reboot and boot wait can hang or take longer than expected.  
  **Mitigation:** Add timeout support and use constants for wait limits.

- **Risk:** Boot-complete parameter may vary by device family.  
  **Mitigation:** Use the observed `bootevent.boot.completed` signal for this framework's target devices and fail clearly on timeout.

- **Risk:** Log dumping after a failed AppFlow can also fail.  
  **Mitigation:** Prefer combined errors or preserve the original AppFlow failure.

- **Risk:** All-window logs include system noise.  
  **Mitigation:** Clear logs before each AppFlow and leave a code comment documenting the future `-P <pid>` filter option.

- **Risk:** `power-shell timeout -o 60000000` changes device state.  
  **Mitigation:** Run it intentionally for every benchmark to prevent lock-related run instability.

## Open Questions

None.
