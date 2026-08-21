## 1. Filename and Documentation

- [x] 1.1 Restore `output_path(now: datetime)` as the single recorder output filename helper using `flow-YYYYMMDD_HHMMSS_microseconds.json`.
- [x] 1.2 Update `main()` to call `output_path(datetime.now())` instead of inlining timestamp formatting.
- [x] 1.3 Update README recorder output filename documentation to the underscore + microseconds format.
- [x] 1.4 Remove the dedicated recorder filename-format unit test.

## 2. Recorder Lifecycle Simplification

- [x] 2.1 Remove `RecorderStartupCapture`, `RecorderStartupSnapshot`, `RecorderProcess`, and `RecordedDeviceData` from `record.py`.
- [x] 2.2 Implement `_read_recorder_startup(process) -> Bounds` to parse hidden recorder stdout readiness/bounds and stop the process on startup failure.
- [x] 2.3 Refactor `convert_ui_inputs_into_flow(hdc, timeout)` into the linear end-to-end recorder orchestration function returning `Flow`.
- [x] 2.4 Keep named structured types where still useful: `Bounds` and `AppInfo`.
- [x] 2.5 Ensure raw HDC recorder stdout remains hidden from normal stdout.
- [x] 2.6 Parse `windowBounds` from recorder stdout as the only source for coordinate normalization bounds.

## 3. Conversion Refactor

- [x] 3.1 Raise `ValueError` for malformed supported recorder records.
- [x] 3.2 Create pointer handlers for tap-like, swipe-like, and home/back pointer operations using signature `Callable[[UIEvent, Bounds], Command]`.
- [x] 3.3 Add `_POINTER_HANDLERS` mapping recorder `OP_TYPE` values to pointer handler functions.
- [x] 3.4 Refactor key conversion into `_handle_key(raw: UIEvent) -> Command | None` without a bounds parameter.
- [x] 3.5 Add `_convert_event(raw, bounds) -> Command | None` to dispatch by `EVENT_TYPE`, use the pointer handler map, print immediate warnings for skipped rows, and let `ValueError` propagate.
- [x] 3.6 Remove warning accumulation from `_convert_events`, `_flow_from_records`, and `main()`.
- [x] 3.7 Keep malformed supported records fatal while unsupported pointer operations and unknown key codes produce no command and print warnings immediately.

## 4. Test Updates

- [x] 4.1 Update `_convert_events` tests to remove the warnings list parameter.
- [x] 4.2 Remove assertions that inspect warning text; verify skipped rows by command absence or empty generated flow instead.
- [x] 4.3 Update integration-style recorder tests to call `convert_ui_inputs_into_flow` or the final retained orchestration boundary instead of removed helpers.
- [x] 4.4 Preserve malformed supported record tests by asserting `ValueError`.
- [x] 4.5 Keep tests verifying empty bundle/ability preservation through generated `AppFlow` fields and labels.

## 5. Verification

- [x] 5.1 Run the project full test target.
- [x] 5.2 Run lint/typecheck commands used by the project.
- [x] 5.3 Run `openspec validate "clean-up-ui-recorder" --type change`.
- [x] 5.4 Run `openspec validate --all`.
