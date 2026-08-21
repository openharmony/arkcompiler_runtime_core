## Why

Run-wide hilog streaming currently depends on Unix-only process group operations such as `os.setsid()` and `os.killpg()`, which prevents the framework from being platform-independent. The logging semantics should remain run-wide and continuous, but the host-side process lifecycle must use portable Python APIs.

## What Changes

- Replace the hilog stream lifecycle implementation based on `multiprocessing.Process`, `os.setsid()`, and process-group signals with direct generic child process handles.
- Keep current run-wide logging semantics: one blocking `hilog > <remote_path>` stream, one `logs/hilog.log` artifact, started before AppFlows and stopped during finalization.
- Move stream process ownership closer to the HDC/device layer so the actual host subprocess running `hdc shell hilog ...` can be terminated directly.
- Use Python built-in cross-platform subprocess APIs such as `subprocess.Popen.terminate()`, `wait(timeout=...)`, and `kill()` without adding wrapper classes around Python processes.
- Preserve finalization behavior: stopping the stream is attempted on success and failure, pending log artifact receive still happens, and stream stop errors are combined with benchmark errors.
- Do not revert to pre-run-wide `hilog -r` / `hilog -x` per-AppFlow logging.

## Capabilities

### New Capabilities

### Modified Capabilities
- `device-hdc`: Device/HDC support for run-wide hilog streaming uses a portable managed subprocess lifecycle instead of Unix process groups.
- `log-streaming`: Run-wide hilog stream lifecycle remains the same semantically, but stream stop requirements no longer require stopping a Unix process group.
- `testing-support`: Tests cover portable stream start/stop behavior without relying on Unix-only signals or process groups.

## Impact

- Affected code: `src/runner.py`, `src/device.py`, `src/hdc.py`, execution context process-handle storage, fake device/HDC helpers, and stream lifecycle tests.
- Removes Unix-only imports/usages from hilog lifecycle code: `os.setsid`, `os.killpg`, and signal-based process-group termination.
- No external dependencies are expected.
- No user-facing CLI or output-layout change.
