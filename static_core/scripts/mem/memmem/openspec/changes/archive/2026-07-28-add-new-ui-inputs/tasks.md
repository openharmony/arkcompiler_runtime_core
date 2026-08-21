## 1. Schema Models

- [x] 1.1 Add strict normalized coordinate, velocity, and step-length type aliases in `src/schema.py`.
- [x] 1.2 Add `TapPayload`, `SwipePayload`, `FlingPayload`, `DirectionalFlingPayload`, `InputTextPayload`, `TextPayload`, and `NamedKeyPayload` models.
- [x] 1.3 Add command models for `tap`, `double_tap`, `long_tap`, `swipe`, `drag`, `fling`, `directional_fling`, `input_text`, and `text`.
- [x] 1.4 Replace loose `KeyCommand` payload validation with named keys only: `Home`, `Back`, and `Power`.
- [x] 1.5 Extend the discriminated `Command` union with all new command models.
- [x] 1.6 Add schema tests covering valid new commands, invalid coordinate percentiles, invalid velocity bounds, missing and invalid step lengths, invalid named keys, numeric key-code rejection, and unknown actions.

## 2. Geometry and Device Operations

- [x] 2.1 Add lean `ScreenBounds` and `Point` dataclasses in `src/device.py` or an existing appropriate module.
- [x] 2.2 Implement `Point.from_normalized(bounds, x_pct, y_pct)` with root-bounds-aware conversion and positive in-bounds clamping.
- [x] 2.3 Implement private `Device._dump_layout()` using `uitest dumpLayout` to a device-local temporary path and receiving the JSON for parsing.
- [x] 2.4 Implement public `Device.screen_bounds()` that parses root `attributes.bounds` and fails clearly when layout dumping or parsing fails.
- [x] 2.5 Add device UI operation methods for tap, double tap, long tap, swipe, drag, fling, directional fling, input text, focused text, and named key.
- [x] 2.6 Add focused tests for bounds parsing, normalized point conversion, direction mapping, and UITest command argument construction.

## 3. Command Execution

- [x] 3.1 Add `screen_bounds: ScreenBounds` to `ExecutionContext`.
- [x] 3.2 Extend command dispatch in `src/commands.py` for all new UI command models.
- [x] 3.3 Convert normalized tap-like coordinates via `Point.from_normalized` before calling device tap, double tap, long tap, and input text methods.
- [x] 3.4 Convert normalized swipe-like start and end coordinates via two `Point.from_normalized` calls before calling device swipe, drag, and fling methods.
- [x] 3.5 Delegate directional fling, text, and named key commands with schema-validated payload values.
- [x] 3.6 Add command tests covering normalized conversion and delegation for every new command action.

## 4. Runner Integration

- [x] 4.1 Call `device.screen_bounds()` after flow validation and device construction but before launching any `AppFlow`.
- [x] 4.2 Store the returned bounds in `ExecutionContext`.
- [x] 4.3 Ensure screen bounds discovery failure aborts before app launch.
- [x] 4.4 Add runner tests verifying screen bounds are read before launch and malformed or failed bounds discovery prevents app launch.

## 5. Validation

- [x] 5.1 Run `source ".venv/bin/activate" && make tests_full` and ensure it passes.
- [x] 5.2 Run `openspec validate --change "add-new-ui-inputs"` and ensure it passes.
