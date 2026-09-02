## 1. Flow Schema

- [x] 1.1 Add required `terminate: bool` to `AppFlow` schema.
- [x] 1.2 Add `screenshot` command schema with label payload validation.
- [x] 1.3 Enforce screenshot labels are unique among screenshot commands.
- [x] 1.4 Keep snapshot and screenshot label uniqueness independent from each other.
- [x] 1.5 Update schema tests for required terminate, screenshot command loading, invalid screenshot label, duplicate screenshot label, and shared snapshot/screenshot label acceptance.

## 2. Device Operations

- [x] 2.1 Add `Device.terminate_app(bundle)` using `aa force-stop <bundle>`.
- [x] 2.2 Make termination succeed only when output contains `force stop process successfully` and fail on HDC errors, `error:` output, or missing success marker.
- [x] 2.3 Add `Device.capture_screenshot(remote_path)` using `uitest screenCap -p <remote_path>`.
- [x] 2.4 Add device-layer tests for termination command translation and strict success/failure parsing.
- [x] 2.5 Add device-layer tests for screenshot capture command translation and failure reporting.

## 3. Screenshot Results and Metadata

- [x] 3.1 Add screenshot metadata models with `screenshots` array, label, timestamp, and `artifacts: list[RelativeParts]` in `src/metadata.py`.
- [x] 3.2 Add local and remote screenshot directory/path helpers to `ResultStore` using `screenshots/<screenshot_label>.png`.
- [x] 3.3 Add screenshots to execution context and metadata writing.
- [x] 3.4 Initialize local and remote `screenshots/` directories for every run.
- [x] 3.5 Add screenshot artifacts to pending artifact receive groups.

## 4. Command Execution

- [x] 4.1 Implement `execute_screenshot()` to timestamp, capture current screen, store metadata, and track the pending screenshot artifact.
- [x] 4.2 Dispatch `ScreenshotCommand` from `execute_command()`.
- [x] 4.3 Add command tests for screenshot capture, metadata, and artifact paths.
- [x] 4.4 Add receive/report tests showing screenshot artifacts are received and reports ignore screenshots.

## 5. AppFlow Termination Lifecycle

- [x] 5.1 Extend runner post-AppFlow handling to attempt termination after any enabled hilog dump when `terminate` is true.
- [x] 5.2 Ensure successful requested termination removes or invalidates the app process in fake-device tests.
- [x] 5.3 Ensure termination failure fails the benchmark when AppFlow commands succeed.
- [x] 5.4 Ensure termination failure combines with an existing AppFlow failure when both occur.
- [x] 5.5 Ensure no termination is attempted when `terminate` is false.

## 6. Fake Device and Documentation

- [x] 6.1 Extend `test/mock/device.py` with termination and screenshot support plus failure knobs.
- [x] 6.2 Update README flow schema and output documentation for required `terminate` and screenshot evidence.
- [x] 6.3 Update sample/test flow JSON fixtures to include explicit `terminate`.
- [x] 6.4 Update delta specs if implementation behavior changes from this plan.

## 7. Verification

- [x] 7.1 Run `source ".venv/bin/activate" && make test`.
- [x] 7.2 Run `source ".venv/bin/activate" && make tests_full`.
- [x] 7.3 Run `openspec validate "add-termination-and-screenshots"`.
