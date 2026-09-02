## MODIFIED Requirements

### Requirement: Flow JSON defines ordered app flows
The system SHALL load and validate a JSON flow file containing an ordered list of `AppFlow` entries with `label`, `bundle`, `ability`, and `commands` fields. Commands MUST be validated as an action-discriminated union before execution. App labels and snapshot labels MUST match `^[A-Za-z0-9_-]+$`. App labels MUST be globally unique across the flow. Snapshot labels MUST be globally unique across all `snapshot` commands in the flow. UI coordinate command payloads MUST use integer percent coordinates from `0` to `100`.

#### Scenario: Valid flow loads successfully
- **WHEN** a flow JSON file contains valid `AppFlow` entries with unique app labels and unique snapshot labels
- **THEN** the system loads the flow and preserves the declared order

#### Scenario: Required field is missing
- **WHEN** a flow JSON file omits a required `AppFlow` field
- **THEN** the system fails validation before performing device actions

#### Scenario: Invalid JSON is provided
- **WHEN** the flow path points to malformed JSON
- **THEN** the system fails validation before performing device actions

#### Scenario: App label contains unsafe characters
- **WHEN** an `AppFlow` label contains characters outside letters, digits, underscore, and hyphen
- **THEN** the system fails validation before performing device actions

#### Scenario: Snapshot label contains unsafe characters
- **WHEN** a `snapshot` command payload contains characters outside letters, digits, underscore, and hyphen
- **THEN** the system fails validation before performing device actions

#### Scenario: Duplicate app label is present
- **WHEN** multiple `AppFlow` entries use the same label
- **THEN** the system fails validation before performing device actions

#### Scenario: Duplicate snapshot label is present
- **WHEN** multiple `snapshot` commands anywhere in the flow use the same payload label
- **THEN** the system fails validation before performing device actions

#### Scenario: UI coordinate is outside normalized range
- **WHEN** a UI command payload contains a percent coordinate less than `0` or greater than `100`
- **THEN** the system fails schema validation before performing device actions

### Requirement: AppFlow launch stores label identity and PID metadata
The system SHALL launch each `AppFlow` once when that flow begins, resolve the launched application's current PID immediately, and store app label as the logical identity with PID, bundle, and ability metadata. Multiple unique app labels MAY resolve to the same PID when the device foregrounds an already running process.

#### Scenario: PID resolves after launch
- **WHEN** an `AppFlow` launch succeeds and a PID is resolved
- **THEN** the system stores the unique app label and associated PID, bundle, and ability metadata as a tracked launched entry

#### Scenario: PID cannot be resolved after launch
- **WHEN** an `AppFlow` launch completes but no PID can be resolved
- **THEN** the system fails the benchmark before executing that flow's commands

#### Scenario: Duplicate labels are present
- **WHEN** multiple `AppFlow` entries use the same label
- **THEN** the system rejects the flow during validation before launching applications

#### Scenario: Duplicate bundle and ability are present
- **WHEN** multiple `AppFlow` entries use the same bundle and ability with different labels
- **THEN** the system allows them and associates each label with the current PID resolved after its launch

### Requirement: Commands execute with action-specific semantics
The system SHALL support command actions `wait`, `snapshot`, `key`, `tap`, `double_tap`, `long_tap`, `swipe`, `drag`, `fling`, `directional_fling`, `input_text`, and `text`, and schema validation MUST enforce each action's payload type before execution.

#### Scenario: Wait command executes
- **WHEN** a `wait` command has a non-negative integer payload
- **THEN** the system waits for the requested number of seconds before proceeding

#### Scenario: Snapshot command executes
- **WHEN** a `snapshot` command has a valid globally unique string payload
- **THEN** the system records one snapshot timestamp, captures smaps on device for every previously launched label whose stored PID is still valid, and records pending snapshot artifacts for later transfer

#### Scenario: Key command executes
- **WHEN** a `key` command has a valid named key payload
- **THEN** the system delegates the key press to the device layer

#### Scenario: Tap command executes
- **WHEN** a `tap` command has `x_pct` and `y_pct` integer payload values in `0..100`
- **THEN** the system converts the normalized coordinate to a cached screen pixel point and delegates a tap to the device layer

#### Scenario: Double tap command executes
- **WHEN** a `double_tap` command has `x_pct` and `y_pct` integer payload values in `0..100`
- **THEN** the system converts the normalized coordinate to a cached screen pixel point and delegates a double tap to the device layer

#### Scenario: Long tap command executes
- **WHEN** a `long_tap` command has `x_pct` and `y_pct` integer payload values in `0..100`
- **THEN** the system converts the normalized coordinate to a cached screen pixel point and delegates a long tap to the device layer

#### Scenario: Swipe command executes
- **WHEN** a `swipe` command has normalized start and end coordinates and a velocity in `200..40000`
- **THEN** the system converts both normalized coordinates to cached screen pixel points and delegates a swipe to the device layer

#### Scenario: Drag command executes
- **WHEN** a `drag` command has normalized start and end coordinates and a velocity in `200..40000`
- **THEN** the system converts both normalized coordinates to cached screen pixel points and delegates a drag to the device layer

#### Scenario: Fling command executes
- **WHEN** a `fling` command has normalized start and end coordinates, a velocity in `200..40000`, and a positive step length
- **THEN** the system converts both normalized coordinates to cached screen pixel points and delegates a fling to the device layer

#### Scenario: Directional fling command executes
- **WHEN** a `directional_fling` command has a direction, a velocity in `200..40000`, and a positive step length
- **THEN** the system delegates a directional fling to the device layer

#### Scenario: Input text command executes
- **WHEN** an `input_text` command has a normalized coordinate and non-empty text
- **THEN** the system converts the normalized coordinate to a cached screen pixel point and delegates coordinate-based text input to the device layer

#### Scenario: Text command executes
- **WHEN** a `text` command has non-empty text
- **THEN** the system delegates focused text input to the device layer

#### Scenario: Unknown command is present
- **WHEN** a command action is not supported
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Velocity is outside UITest range
- **WHEN** a swipe-like command payload contains velocity less than `200` or greater than `40000`
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Step length is missing for fling
- **WHEN** a `fling` or `directional_fling` command omits `step_length`
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Step length is not positive
- **WHEN** a `fling` or `directional_fling` command contains `step_length` less than or equal to zero
- **THEN** the system fails schema validation before performing device actions

### Requirement: Snapshot skips invalid stored PIDs
The system SHALL skip a tracked launched label during snapshot collection when that label's stored `/proc/<pid>` no longer exists.

#### Scenario: Stored PID is missing during snapshot
- **WHEN** a snapshot command inspects a tracked label whose stored PID path `/proc/<pid>` is missing
- **THEN** the system skips that label for that snapshot without failing solely for that reason

#### Scenario: Stored PID exists during snapshot
- **WHEN** a snapshot command inspects a tracked label whose stored PID path `/proc/<pid>` exists
- **THEN** the system writes `/proc/<pid>/smaps` to `snapshots/<snapshot_label>/<app_label>.smaps` as a device-local snapshot artifact and records it for later transfer

#### Scenario: All stored PIDs are missing during snapshot
- **WHEN** a snapshot command finds no tracked labels with existing stored PID paths
- **THEN** the system still records snapshot metadata for that snapshot and records no pending artifacts

#### Scenario: App relaunches outside framework control
- **WHEN** an app relaunches with a different PID outside framework control
- **THEN** the system does not rediscover the new PID and continues using only stored PIDs
