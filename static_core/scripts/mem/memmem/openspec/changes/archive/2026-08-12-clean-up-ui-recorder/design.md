## Context

`record.py` was introduced as a root-level helper that starts `uitest uiRecord`, reads `/data/local/tmp/record.csv`, converts recorder events to benchmark flow commands, and writes a generated flow JSON file. Recent feedback identified three cleanup areas: output filenames should match the `run.py` timestamp style, recorder lifecycle wrappers make the script harder to follow, and event conversion should isolate case-specific parsing instead of using nested branching with accumulated warnings.

The current working tree already contains uncommitted recorder edits. This change intentionally builds on top of that diff rather than reverting it.

## Goals / Non-Goals

**Goals:**

- Align recorder output filenames with `run.py` by using `flow-YYYYMMDD_HHMMSS_microseconds.json`.
- Remove low-value filename-format testing instead of adding test surface for a simple timestamp string.
- Simplify recorder lifecycle code by removing redundant startup/process wrapper dataclasses.
- Keep named dataclasses where structured data remains useful instead of returning nameless tuples.
- Refactor conversion into separate pointer and key processing paths.
- Dispatch pointer conversion through an `OP_TYPE` handler map.
- Print warnings immediately to stderr for skipped unsupported records and empty generated app identities.
- Keep malformed supported records fatal.

**Non-Goals:**

- Do not change the flow schema or command schema.
- Do not add alternative recording sources or improve `uiRecord` capture reliability.
- Do not infer waits, screenshots, snapshots, text semantics, or app identity fallback.
- Do not preserve warning accumulation as part of programmatic API behavior.

## Decisions

### 1. Filename helper restored, test removed

The implementation should keep an `output_path(now: datetime) -> pathlib.Path` helper so `main()` does not inline timestamp formatting. The helper should produce `flow-{now.strftime('%Y%m%d_%H%M%S_%f')}.json`, matching the underscore + microseconds convention used by `run.py` output directories.

The previous filename unit test should be removed because it only duplicated formatting behavior and created churn when the convention changed. Specs and README are sufficient to document the convention.

### 2. Remove redundant recorder lifecycle dataclasses

The following dataclasses should be removed:

- `RecorderStartupCapture`
- `RecorderStartupSnapshot`
- `RecorderProcess`
- `RecordedDeviceData`

`Bounds` and `AppInfo` should remain because they provide useful named structure. `convert_ui_inputs_into_flow` should return `Flow` directly; warnings are no longer accumulated.

Recorder startup should become one function that starts a background stdout reader, waits for the readiness event, captures validated bounds from stdout, and keeps raw HDC output hidden. Coordinate bounds come only from recorder stdout:

```text
_read_recorder_startup(process) -> Bounds
  ├─ background thread reads process.stdout
  ├─ parse first windowBounds line
  ├─ signal readiness on Started Recording Successfully...
  ├─ fail on readiness timeout
  ├─ fail on missing/invalid bounds
  └─ return Bounds
```

`convert_ui_inputs_into_flow(hdc, timeout)` should be the linear orchestration function:

```text
process = hdc.start_shell(..., stdout=PIPE, stderr=PIPE)
bounds = _read_recorder_startup(process)
prompt user
wait for manual/timeout/process stop
stop recorder
read record.csv
parse rows
convert rows
build validated Flow
return flow
```

This avoids wrapper objects around short-lived startup state or the high-level flow result.

### 3. Use separate pointer and key conversion paths

Pointer conversion should use an `OP_TYPE` dispatch map:

```text
_POINTER_HANDLERS = {
  "click": _handle_pointer_tap,
  "doubleClick": _handle_pointer_tap,
  "longClick": _handle_pointer_tap,
  "swipe": _handle_pointer_swipe,
  "drag": _handle_pointer_swipe,
  "fling": _handle_pointer_swipe,
  "home": _handle_pointer_home_back,
  "back": _handle_pointer_home_back,
}
```

Pointer handlers should use signature:

```python
Callable[[UIEvent, Bounds], Command]
```

They are only called after `OP_TYPE` lookup succeeds, so unsupported matching does not belong inside handlers. Each handler should parse only its own supported case and raise `ValueError` for malformed supported input.

Key conversion should stay separate because key events do not require bounds and do not dispatch on `OP_TYPE`:

```python
def _handle_key(raw: UIEvent) -> Command | None:
```

Unknown supported key events should return `None` after the caller prints a warning. Malformed key records should raise `ValueError`.

### 4. Immediate warnings and fatal malformed supported records

Warnings should not be accumulated. Skipped unsupported records and empty generated app identities should print immediately to stderr. This removes warning lists from `_convert_events` and `_convert_event`.

`ValueError` means the recorder produced an event shape that cannot be parsed safely, including malformed JSON, unknown event types rejected by the typed schema, or malformed supported records. It should propagate and abort conversion. Unsupported pointer `OP_TYPE` and unknown key code are not exceptions; they print a warning and produce no command.

### 5. Tests focus on behavior, not warning text

Tests that previously asserted warning contents should instead verify structural behavior:

- Unsupported pointer rows produce no command.
- Unknown key rows produce no command while known key rows still convert.
- Empty bundle/ability values remain empty in generated `AppFlow` entries.
- Malformed supported rows still raise `ValueError`.

The filename-format unit test should be removed.

## Risks / Trade-offs

- **Warnings become harder to assert exactly** → Tests intentionally avoid warning text assertions and verify behavioral outcomes instead.
- **Immediate warning printing may interleave with user-facing progress output** → All warnings should print to stderr with the existing `warning:` prefix to keep stdout focused on normal progress and generated filename.
- **Removing startup wrapper dataclasses reduces explicit state objects** → The startup lifecycle becomes easier to read; keep `_read_recorder_startup` small and single-purpose.
- **Pointer handler map can hide control flow if over-abstracted** → Use direct function names and a simple dict; avoid generic class-based handler frameworks.
- **Unknown key warnings need keycode context** → `_convert_event` should print `raw.key_code_1` when `_handle_key` returns `None`.
