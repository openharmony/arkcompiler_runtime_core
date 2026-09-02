## 1. CLI and Recording Lifecycle

- [x] 1.1 Create root-level `record.py` with argparse support for optional positive float `--timeout`.
- [x] 1.2 Reuse or mirror `.env` `HDC_PATH` loading so the recorder uses the configured HDC executable.
- [x] 1.3 Start `hdc shell "uitest uiRecord record -W false -c true"` as a subprocess, parse recorder startup output internally, and keep raw HDC output off stdout.
- [x] 1.4 Detect recorder readiness from `Started Recording Successfully...` with an internal 120-second startup timeout.
- [x] 1.5 Implement manual stop prompt accepting `y` + Enter.
- [x] 1.6 Implement timeout-based stop that starts after readiness and first-stop-wins coordination between timeout and manual confirmation.
- [x] 1.7 Stop the recorder subprocess by sending SIGINT and treat the observed recorder stop return code as normal only after requested stop.
- [x] 1.8 Retrieve device `/data/local/tmp/record.csv` after recording stops.

## 2. Record Parsing and Conversion

- [x] 2.1 Parse recorder bounds from startup stdout or optional `windowBounds` row and validate nonzero width/height bounds.
- [x] 2.2 Parse JSON event rows through Pydantic recorder-event models, fail conversion on malformed supported input, and collect warnings only for unsupported events.
- [x] 2.3 Normalize raw recorder coordinates into clamped integer percentages.
- [x] 2.4 Convert `click`, `doubleClick`, and `longClick` records into `tap`, `double_tap`, and `long_tap` commands without adding waits.
- [x] 2.5 Convert `swipe`, `drag`, and `fling` records into corresponding swipe-like commands with rounded/clamped strict integer velocity and positive step length handling.
- [x] 2.6 Convert supported Home/Back/Power key records into `key` commands and skip unknown keycodes with warnings.
- [x] 2.7 Preserve input order by recorded bundle/ability identity segments, leave empty bundle or ability values empty, and warn users about empty identity segments.
- [x] 2.8 Skip unsupported recorder events with warnings, preserve typed text as pointer commands when recorder emits pointer events, and fail if no supported commands remain.

## 3. Flow Serialization

- [x] 3.1 Build a canonical `Flow` containing generated-label `AppFlow` entries with recorded bundle/ability identity segments, `terminate: false`, and recorded input commands only.
- [x] 3.2 Validate generated flow through the existing Pydantic schema before writing.
- [x] 3.3 Serialize to `flow-YYYYMMDD-HHMMSS-MILLISECONDS.json` in the current working directory.
- [x] 3.4 Print the generated filename and any conversion warnings to the user.

## 4. Tests and Documentation

- [x] 4.1 Add unit tests for bounds parsing from startup stdout and coordinate normalization.
- [x] 4.2 Add unit tests for pointer event conversion, velocity/step normalization, and unsupported-event warnings.
- [x] 4.3 Add unit tests for key event conversion and unknown-key skipping.
- [x] 4.4 Add unit tests for app-flow identity-segment ordering, empty-identity preservation, `terminate: false`, label generation, timestamped filename generation, and generated flow validation.
- [x] 4.5 Add CLI tests for invalid timeout, readiness timeout, malformed input failure, and no-supported-command failure.
- [x] 4.6 Add a root README `Record (record.py)` section with usage, limitations, and generated-file editing guidance.
- [x] 4.7 Rename README headings to reference corresponding files: `Run (run.py)`, `Record (record.py)`, and `Programmatic API (lib.py)`.

## 5. Verification

- [x] 5.1 Run the project test suite.
- [x] 5.2 Run lint/typecheck commands used by the project.
- [x] 5.3 Run `openspec validate "add-ui-input-recorder" --type change`.
- [x] 5.4 Run `openspec validate --all`.
