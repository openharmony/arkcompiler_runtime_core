## MODIFIED Requirements

### Requirement: Recorder parses device record CSV
The recorder SHALL parse `/data/local/tmp/record.csv` as JSON event rows through a typed recorder-event schema and SHALL fail the entire conversion on malformed input records rather than producing partial flow JSON from suspect input. The recorder SHALL use bounds captured from recorder startup stdout to normalize pointer coordinates and SHALL preserve recorded app identity values exactly, including empty bundle or ability values.

#### Scenario: Window bounds are parsed
- **WHEN** recorder startup output provides left, top, right, and bottom values in a `windowBounds` line
- **THEN** the system uses those bounds to normalize pointer coordinates into integer percentages

#### Scenario: Missing bounds fails conversion
- **WHEN** recorder startup output does not include valid `windowBounds`
- **THEN** the system exits with an error and does not write a flow file

#### Scenario: Malformed event fails conversion
- **WHEN** a record row has malformed JSON, unknown `EVENT_TYPE`, or an otherwise supported event is missing required fields
- **THEN** the system exits with an error and does not write a flow file

### Requirement: Recorder converts supported events to flow commands
The recorder SHALL convert supported recorded events into existing canonical benchmark command actions, SHALL NOT insert `wait` or evidence commands, SHALL preserve the recorded input order inside generated app-flow segments, SHALL preserve generated app-flow segment order, and SHALL validate the generated `Flow` before writing it. The recorder SHALL treat malformed records and unknown event types as fatal conversion errors. The recorder SHALL print warnings immediately for unsupported pointer operations or skipped key codes and SHALL NOT accumulate warnings for later output.

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
- **THEN** the recorder prints a warning immediately and skips it rather than substituting another key

#### Scenario: Text entry remains UI input
- **WHEN** soft-keyboard text entry appears in the recorder as pointer click or double-click events
- **THEN** the recorder converts those records as pointer commands rather than inferring semantic `text` or `input_text` commands

#### Scenario: Unsupported pointer operation is skipped
- **WHEN** the record contains a pointer event with no equivalent benchmark command
- **THEN** the recorder prints a warning immediately and skips it rather than emitting an invalid command

#### Scenario: Unknown event type is fatal
- **WHEN** the record contains an event whose `EVENT_TYPE` is not supported by the typed recorder-event schema
- **THEN** the recorder exits with an error and does not write a flow file

### Requirement: Recorder writes generated flow file
The recorder SHALL write the generated canonical `Flow` JSON into `flow-YYYYMMDD_HHMMSS_microseconds.json` in the current working directory and print the generated filename to stdout.

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
- **THEN** the generated `AppFlow` uses an empty string for each missing field, sanitizes the generated label, prints a warning immediately, and continues

#### Scenario: No supported commands are recorded
- **WHEN** recording stops but no supported command can be generated
- **THEN** the system exits with an error and does not write an empty flow file

#### Scenario: Generated flow has no termination intent
- **WHEN** the flow file is generated
- **THEN** every `AppFlow` has `terminate: false` because the recorder preserves input order rather than app lifecycle intent

#### Scenario: Generated flow is editable
- **WHEN** the flow file is generated
- **THEN** it contains generated labels, recorded bundle and ability values, `terminate: false`, a recorder description, and converted input commands for the user to edit before benchmarking
