## Why

Benchmark flows need stronger lifecycle and visual evidence controls. Explicit AppFlow termination prevents earlier app launches from contaminating later memory snapshots, while screenshot evidence helps verify that a flow reached the intended UI state when memory evidence was captured.

## What Changes

- **BREAKING**: Require every `AppFlow` to include an explicit `terminate` boolean field; no default is provided.
- If an AppFlow has `terminate: true`, terminate its bundle after command execution and after any AppFlow hilog dump using a `Device.terminate_app()` operation.
- Termination failures fail the benchmark; when an AppFlow already failed, termination failures are combined with the original AppFlow failure as post-flow errors.
- Add a `screenshot` command whose payload is a screenshot label.
- Capture screenshots as PNG files on device using `uitest screenCap -p <remote_path>` behind `Device.capture_screenshot()`.
- Store screenshots under `screenshots/<screenshot_label>.png` without nested label directories.
- Write `screenshot_metadata.json` with screenshot records in command order, each preserving label, timestamp, and `artifacts: list[RelativeParts]`.
- Receive pending screenshot artifacts together with existing snapshot and log artifact groups.
- Keep screenshots as raw evidence only; generated CSV reports do not consume screenshots.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: AppFlow schema requires `terminate`, command schema supports `screenshot`, and AppFlow post-processing supports requested termination.
- `device-hdc`: device layer adds bundle termination and screenshot capture operations.
- `result-evidence`: output layout and metadata include screenshot artifacts and `screenshot_metadata.json`.
- `testing-support`: fake device and fake HDC tests model termination and screenshot capture behavior.

## Impact

- Affected code: `src/schema.py`, `src/commands.py`, `src/device.py`, `src/runner.py`, `src/result.py`, `src/metadata.py`, tests, and README.
- Existing flow JSON files must be updated to include `terminate` on every AppFlow.
- Output directories gain `screenshots/` and `screenshot_metadata.json`.
- Device command translation relies on `aa force-stop <bundle>` for termination and `uitest screenCap -p <path>` for screenshots.
