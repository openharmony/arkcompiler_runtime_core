## ADDED Requirements

### Requirement: Recorder CLI captures UI inputs
The system SHALL provide a root-level `record.py` CLI that records UI input events from a connected OpenHarmony device using device-side `uitest uiRecord record` as the sole recording source and retrieves `/data/local/tmp/record.csv` after recording stops.

#### Scenario: Recorder starts successfully
- **WHEN** the user runs the recorder CLI and the connected device supports `uitest uiRecord record`
- **THEN** the system starts recording, waits until `Started Recording Successfully...` is observed from raw recorder output without echoing raw HDC output, prints that recording is ready, and prompts the user to perform inputs

#### Scenario: Recorder readiness times out
- **WHEN** the recorder process does not report readiness within 120 seconds
- **THEN** the system exits with an error and does not start the user recording timeout

#### Scenario: Recorder fails to start
- **WHEN** the connected device does not support `uitest uiRecord record` or HDC cannot start the command
- **THEN** the system exits with an error and does not write a flow file

#### Scenario: Existing device recording file is replaced
- **WHEN** the recorder starts a new session
- **THEN** the device-side `/data/local/tmp/record.csv` content for that session is used as the source of generated commands

### Requirement: Recorder CLI supports timeout and manual stop
The recorder CLI SHALL accept an optional named `--timeout` float in seconds and SHALL stop recording when either the timeout elapses after recorder readiness or the user confirms completion through stdin.

#### Scenario: Timeout stops recording
- **WHEN** the user starts recording with a timeout and does not manually stop before the timeout elapses after recorder readiness
- **THEN** the system sends SIGINT to the recorder process and proceeds to read recorded events

#### Scenario: Manual confirmation stops recording
- **WHEN** the user starts recording and types `y` followed by Enter at the completion prompt
- **THEN** the system sends SIGINT to the recorder process and proceeds to read recorded events

#### Scenario: Manual stop wins before timeout
- **WHEN** the user starts recording with a timeout and confirms completion before the timeout elapses after readiness
- **THEN** the system sends SIGINT immediately and does not wait for the timeout

#### Scenario: Invalid timeout fails
- **WHEN** the user provides a timeout that is not a positive float
- **THEN** the CLI exits with argument validation error before starting device recording

### Requirement: Recorder parses device record CSV
The recorder SHALL parse `/data/local/tmp/record.csv` as JSON event rows through a typed recorder-event schema and SHALL fail the entire conversion on malformed input records rather than producing partial flow JSON from suspect input. The recorder SHALL use bounds captured from recorder startup stdout, or a `windowBounds` CSV row if present, to normalize pointer coordinates and SHALL preserve recorded app identity values exactly, including empty bundle or ability values.

#### Scenario: Window bounds are parsed
- **WHEN** recorder startup output or the record file provides left, top, right, and bottom values
- **THEN** the system uses those bounds to normalize pointer coordinates into integer percentages

#### Scenario: Missing bounds fails conversion
- **WHEN** the record file does not include a valid `windowBounds` row
- **THEN** the system exits with an error and does not write a flow file

#### Scenario: Malformed event fails conversion
- **WHEN** a record row has malformed JSON or an otherwise supported event is missing required fields
- **THEN** the system exits with an error and does not write a flow file

### Requirement: Recorder converts supported events to flow commands
The recorder SHALL convert supported recorded events into existing canonical benchmark command actions, SHALL NOT insert `wait` or evidence commands, SHALL preserve the recorded input order inside generated app-flow segments, SHALL preserve generated app-flow segment order, and SHALL validate the generated `Flow` before writing it.

#### Scenario: Tap-like pointer events convert
- **WHEN** the record contains pointer events with `OP_TYPE` `click`, `doubleClick`, or `longClick`
- **THEN** the generated flow contains corresponding `tap`, `double_tap`, or `long_tap` commands with normalized `x_pct` and `y_pct` payloads

#### Scenario: Swipe-like pointer events convert
- **WHEN** the record contains pointer events with `OP_TYPE` `swipe`, `drag`, or `fling`
- **THEN** the generated flow contains corresponding `swipe`, `drag`, or `fling` commands with normalized start and end coordinates and strict integer velocity values clamped to `200..40000`

#### Scenario: Fling event includes step length
- **WHEN** the record contains a pointer event with `OP_TYPE` `fling`
- **THEN** the generated command payload includes a positive strict integer `step_length`

#### Scenario: Supported key event converts
- **WHEN** the record contains a key event that maps to `Home`, `Back`, or `Power`
- **THEN** the generated flow contains a corresponding `key` command

#### Scenario: Unknown key event is skipped
- **WHEN** the record contains a key event that does not map to `Home`, `Back`, or `Power`
- **THEN** the recorder skips it with a warning rather than substituting another key

#### Scenario: Text entry remains UI input
- **WHEN** soft-keyboard text entry appears in the recorder as pointer click or double-click events
- **THEN** the recorder converts those records as pointer commands rather than inferring semantic `text` or `input_text` commands

#### Scenario: Unsupported event is skipped
- **WHEN** the record contains an event with no equivalent benchmark command
- **THEN** the recorder skips it with a warning rather than emitting an invalid command

### Requirement: Recorder writes generated flow file
The recorder SHALL write the generated canonical `Flow` JSON into `flow-YYYYMMDD-HHMMSS-MILLISECONDS.json` in the current working directory and print the generated filename to stdout.

#### Scenario: Output file is written
- **WHEN** recording stops and at least one supported command is generated
- **THEN** the system writes a valid flow JSON file grouped into one or more recorded bundle/ability `AppFlow` segments and prints its filename

#### Scenario: AppFlow order preserves identity encounter order
- **WHEN** converted records contain inputs for multiple bundle/ability identities over time
- **THEN** generated `AppFlow` entries appear in the same order those identity segments appeared in the recording

#### Scenario: AppFlow labels derive from recorder metadata
- **WHEN** converted records contain bundle and ability metadata
- **THEN** each generated `AppFlow` label is derived from that bundle and ability plus the encounter count for the same pair, using only schema-safe label characters

#### Scenario: Empty app identity is preserved with warning
- **WHEN** an otherwise supported record has empty bundle or ability metadata
- **THEN** the generated `AppFlow` uses an empty string for each missing field, sanitizes the generated label, and reports a warning

#### Scenario: No supported commands are recorded
- **WHEN** recording stops but no supported command can be generated
- **THEN** the system exits with an error and does not write an empty flow file

#### Scenario: Generated flow has no termination intent
- **WHEN** the flow file is generated
- **THEN** every `AppFlow` has `terminate: false` because the recorder preserves input order rather than app lifecycle intent

#### Scenario: Generated flow is editable
- **WHEN** the flow file is generated
- **THEN** it contains generated labels, recorded bundle and ability values, `terminate: false`, a recorder description, and converted input commands for the user to edit before benchmarking
