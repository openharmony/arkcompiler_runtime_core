## Context

`memmem_clean` currently supports `wait`, `snapshot`, and a loose `key` command. HDC source shows that `hdc shell` is a transport layer for shell commands, while UITest owns the `uiInput` command grammar. OpenHarmony UITest exposes stable shell operations for click, double click, long click, swipe, drag, fling, directional fling, text input, and key events.

A device probe using the configured `HDC_PATH` confirmed that `hdc shell uitest dumpLayout -p /data/local/tmp/...` produces layout JSON whose root node includes bounds such as `[0,0][1280,2832]`. This is sufficient to convert normalized flow coordinates into absolute UITest pixel coordinates without recalculating flows per device.

## Goals / Non-Goals

**Goals:**
- Add portable normalized UI input commands to flow schema.
- Use integer percent coordinates from `0` to `100`.
- Read screen bounds once before any `AppFlow` executes and reuse cached bounds for every UI command.
- Keep device APIs lean and backed only by stable `uitest uiInput` shell commands.
- Tighten `key` to named keys only: `Home`, `Back`, and `Power`.
- Validate velocity and step-length inputs before device actions.

**Non-Goals:**
- Do not support multi-display `displayId`; test devices are single-screen.
- Do not support numeric key codes or key combinations in this change.
- Do not support internal-only `ui_action.h` types that lack stable `uiInput` shell bindings.
- Do not add element lookup, selector-based coordinates, or runtime layout refresh per command.
- Do not change smaps reporting or evidence layout.

## Decisions

### Cache root screen bounds once

The runner will call `Device.screen_bounds()` before launching the first `AppFlow`. The device layer will implement this using a private `_dump_layout()` helper and parse the root `attributes.bounds` field.

This fails early if UITest layout dumping is unavailable or malformed, before any app launch mutates device state.

### Use lean geometry structs

Add `ScreenBounds` and `Point` only. `ScreenBounds` contains `left`, `top`, `right`, and `bottom` fields without computed properties. `Point` contains `x` and `y` and exposes `Point.from_normalized(bounds, x_pct, y_pct)` as the only coordinate factory.

`Point.from_normalized` computes width and height locally from bounds, converts integer percentiles to pixels, and clamps to positive in-bounds coordinates accepted by UITest.

### Expose normalized coordinates only

Flow schemas use `x_pct`, `y_pct`, `x1_pct`, `y1_pct`, `x2_pct`, and `y2_pct` integers in `0..100`. Raw pixel coordinates are intentionally excluded to avoid competing coordinate systems and device-specific flows.

### Keep UI command payloads explicit

`TapPayload` is reused by `tap`, `double_tap`, and `long_tap`. `SwipePayload` is reused by `swipe` and `drag`. `FlingPayload` extends swipe semantics with mandatory `step_length`. `DirectionalFlingPayload` uses `direction`, mandatory `velocity`, and mandatory `step_length`.

Velocity is mandatory and strictly validated as `200..40000`, matching UITest's documented valid range. `step_length` is mandatory for fling commands and validated as a positive integer. The framework will not calculate swipe distance or validate `step_length <= distance`; invalid runtime geometry remains delegated to UITest.

### Keep key support named-only

Replace loose dictionary key payloads with `NamedKeyPayload` containing `Home`, `Back`, or `Power`. The device API remains `send_key(key: str)` to avoid coupling device code to schema models.

### Device API mirrors supported UITest commands

Add public device operations for the supported UI commands and keep `_dump_layout()` private. Device methods translate framework names to UITest command names, for example `tap` to `click` and `long_tap` to `longClick`.

## Risks / Trade-offs

- Cached bounds can become stale if display geometry changes during a run → Treat this as out of scope for single-screen benchmark devices and fail early only on initial bounds discovery.
- Percent coordinates are less precise than pixels → This is acceptable for portable benchmark flows; users can choose integer percentages carefully.
- Raw layout JSON is not preserved as evidence → The change only needs bounds for execution; preserving layout evidence can be added later if needed.
- Named-only keys exclude shortcuts and text-like key injection → Numeric key combinations are deferred to keep schema and validation small.
