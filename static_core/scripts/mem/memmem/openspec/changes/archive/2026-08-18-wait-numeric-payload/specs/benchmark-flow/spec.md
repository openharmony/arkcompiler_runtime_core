## MODIFIED Requirements

### Requirement: Commands execute with action-specific semantics
The system SHALL support command actions `wait`, `snapshot`, `screenshot`, `key`, `tap`, `double_tap`, `long_tap`, `swipe`, `drag`, `fling`, `directional_fling`, `input_text`, and `text`, and schema validation MUST enforce each action's payload type before execution.

#### Scenario: Wait command executes
- **WHEN** a `wait` command has a non-negative numeric payload (integer or float)
- **THEN** the system waits for the requested number of seconds before proceeding

#### Scenario: Wait command rejects invalid payloads
- **WHEN** a `wait` command payload is negative or non-numeric
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Snapshot command executes
- **WHEN** a `snapshot` command has a valid globally unique string payload
- **THEN** the system records one snapshot timestamp, captures smaps on device for every previously launched label whose stored PID is still valid, and records pending snapshot artifacts for later transfer

#### Scenario: Screenshot command executes
- **WHEN** a `screenshot` command has a valid globally unique screenshot label payload
- **THEN** the system records one screenshot timestamp, captures the current screen to a device-local PNG file, and records a pending screenshot artifact for later transfer

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