## MODIFIED Requirements

### Requirement: Flow JSON defines ordered app flows
The system SHALL load and validate a JSON flow file containing an ordered list of `AppFlow` entries with `label`, `bundle`, `ability`, and `commands` fields. Commands MUST be validated as an action-discriminated union before execution. App labels and snapshot labels MUST match `^[A-Za-z0-9_-]+$`. UI coordinate command payloads MUST use integer percent coordinates from `0` to `100`.

#### Scenario: Valid flow loads successfully
- **WHEN** a flow JSON file contains valid `AppFlow` entries
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

#### Scenario: UI coordinate is outside normalized range
- **WHEN** a UI command payload contains a percent coordinate less than `0` or greater than `100`
- **THEN** the system fails schema validation before performing device actions

### Requirement: Commands execute with action-specific semantics
The system SHALL support command actions `wait`, `snapshot`, `key`, `tap`, `double_tap`, `long_tap`, `swipe`, `drag`, `fling`, `directional_fling`, `input_text`, and `text`, and schema validation MUST enforce each action's payload type before execution.

#### Scenario: Wait command executes
- **WHEN** a `wait` command has a non-negative integer payload
- **THEN** the system waits for the requested number of seconds before proceeding

#### Scenario: Snapshot command executes
- **WHEN** a `snapshot` command has a valid string payload
- **THEN** the system captures smaps on device for every previously launched process whose stored PID is still valid and records pending snapshot artifacts for later transfer

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

### Requirement: Benchmark execution caches screen bounds before app flows
The system SHALL read root screen bounds from the device before executing any `AppFlow` and store those bounds in the execution context for normalized coordinate conversion.

#### Scenario: Screen bounds are read successfully
- **WHEN** the benchmark begins execution after flow validation
- **THEN** the system reads device layout, parses root bounds, stores them in execution context, and only then launches the first `AppFlow`

#### Scenario: Screen bounds cannot be read
- **WHEN** device layout dumping fails before any `AppFlow` starts
- **THEN** the system fails the benchmark before launching any application

#### Scenario: Screen bounds are malformed
- **WHEN** the root layout does not contain parseable bounds
- **THEN** the system fails the benchmark before launching any application

#### Scenario: Normalized coordinate is converted
- **WHEN** a UI command requests coordinate conversion for `x_pct` and `y_pct`
- **THEN** the system converts the percent coordinate using cached root bounds and clamps the result to a positive in-bounds pixel coordinate

### Requirement: Key command accepts named keys only
The system SHALL accept `key` commands only when the payload specifies one named key from `Home`, `Back`, or `Power`.

#### Scenario: Named key is valid
- **WHEN** a `key` command payload contains `key` equal to `Home`, `Back`, or `Power`
- **THEN** the system accepts the command and delegates the named key to the device layer

#### Scenario: Named key is invalid
- **WHEN** a `key` command payload contains any other key value
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Numeric key codes are provided
- **WHEN** a `key` command payload contains numeric key codes or key combinations
- **THEN** the system fails schema validation before performing device actions
