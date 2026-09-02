## Why

Current tests duplicate fake device and HDC behavior across files, making scenarios harder to read and increasing the chance that fake behavior drifts from framework semantics. `run_benchmark()` also constructs its own device from `Config`, forcing runner tests to monkeypatch production construction instead of passing a test device directly.

## What Changes

- Add shared test support fakes under `test/mock/`:
  - `FakeHdc` logs HDC commands and returns configured `HdcResult` values.
  - `FakeDevice` models device state with configured screen bounds, bundle-to-PID processes, invalid bundles, remote directories, remote files, and failure knobs.
- Refactor `run.py` to construct `Config`, `Hdc`, and `Device`, then pass the device to benchmark execution.
- **BREAKING** for internal callers: change `run_benchmark(flow_path, out_dir, config)` to `run_benchmark(flow_path, out_dir, device)`.
- Refactor tests to use shared fakes and direct dependency injection instead of local fake classes, class-level fake state, and `mock.patch("src.runner.Device", ...)`.
- Move command-translation assertions toward `Device`/`FakeHdc` tests while using `FakeDevice` for stateful runner and command orchestration tests.

## Capabilities

### New Capabilities
- `testing-support`: Shared testing fakes and dependency-injection seams used by framework tests.

### Modified Capabilities
- `benchmark-flow`: Internal benchmark execution API accepts an already-created device instead of constructing one from configuration.

## Impact

- Affected code: `run.py`, `src/runner.py`, and tests under `test/`.
- New test helper modules: `test/mock/hdc.py` and `test/mock/device.py`.
- Production behavior remains unchanged for CLI users.
- Internal API change: callers of `run_benchmark()` must pass a `Device`-compatible object instead of `Config`.
