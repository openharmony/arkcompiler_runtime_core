## Why

Benchmark flows need portable UI input operations beyond `wait`, `snapshot`, and `key`. Current key support is loose and coordinate-based interactions would require recalculating pixels for each device unless the framework supports normalized coordinates.

## What Changes

- Add normalized UITest-backed flow commands: `tap`, `double_tap`, `long_tap`, `swipe`, `drag`, `fling`, `directional_fling`, `input_text`, and `text`.
- Tighten `key` to named keys only: `Home`, `Back`, and `Power`.
- Read device layout once before any `AppFlow` starts, parse root bounds, and store the bounds in execution context.
- Convert integer percent coordinates to absolute pixels at command execution time using cached root bounds.
- Require strict velocity validation for swipe-like commands using UITest's `200..40000` range.
- Require `step_length` for `fling` and `directional_fling`.
- Discard internal-only `ui_action.h` actions that are not exposed by stable `uitest uiInput` shell commands.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: add normalized UI input command schemas, strict named key schema, and cached screen-bounds execution semantics.
- `device-hdc`: add UITest-backed device operations and screen bounds discovery through `uitest dumpLayout`.

## Impact

- Affects `src/schema.py`, `src/device.py`, `src/commands.py`, and `src/runner.py`.
- Adds or updates schema, command, device, and runner tests.
- No change to raw smaps evidence layout or reporting.
- No new runtime dependency is expected.
