## ADDED Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument and an optional `--out` argument.

#### Scenario: User requests help
- **WHEN** the user runs `python run.py --help`
- **THEN** the system displays command-line help describing `--flow` and `--out`

#### Scenario: User omits flow argument
- **WHEN** the user runs `python run.py` without `--flow`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides output directory
- **WHEN** the user runs `python run.py --flow flow.json --out outdir`
- **THEN** the system uses `outdir` as the benchmark output directory

#### Scenario: User omits output directory
- **WHEN** the user runs `python run.py --flow flow.json`
- **THEN** the system uses a timestamped default output directory

### Requirement: Flow JSON defines ordered app flows
The system SHALL load and validate a JSON flow file containing an ordered list of `AppFlow` entries with `label`, `bundle`, `ability`, and `commands` fields. Commands MUST be validated as an action-discriminated union before execution. App labels and snapshot labels MUST match `^[A-Za-z0-9_-]+$`.

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

### Requirement: AppFlow launch stores PID identity
The system SHALL launch each `AppFlow` once when that flow begins, resolve the launched application's PID immediately, and store the PID as runtime identity with label, bundle, and ability metadata.

#### Scenario: PID resolves after launch
- **WHEN** an `AppFlow` launch succeeds and a PID is resolved
- **THEN** the system stores the PID and associated metadata as a tracked launched process

#### Scenario: PID cannot be resolved after launch
- **WHEN** an `AppFlow` launch completes but no PID can be resolved
- **THEN** the system fails the benchmark before executing that flow's commands

#### Scenario: Duplicate labels are present
- **WHEN** multiple `AppFlow` entries use the same label
- **THEN** the system allows them and tracks each launched process by PID

#### Scenario: Duplicate bundle and ability are present
- **WHEN** multiple `AppFlow` entries use the same bundle and ability
- **THEN** the system allows them and resolves each new PID while excluding PIDs already tracked by the framework

### Requirement: Commands execute with action-specific semantics
The system SHALL support initial command actions `wait`, `snapshot`, and `key`, and schema validation MUST enforce each action's payload type before execution.

#### Scenario: Wait command executes
- **WHEN** a `wait` command has a non-negative integer payload
- **THEN** the system waits for the requested number of seconds before proceeding

#### Scenario: Snapshot command executes
- **WHEN** a `snapshot` command has a valid string payload
- **THEN** the system captures smaps on device for every previously launched process whose stored PID is still valid and records pending snapshot artifacts for later transfer

#### Scenario: Key command executes
- **WHEN** a `key` command has a dictionary string payload valid for the selected key action
- **THEN** the system delegates the key press to the device layer

#### Scenario: Unknown command is present
- **WHEN** a command action is not supported
- **THEN** the system fails schema validation before performing device actions

### Requirement: Snapshot skips invalid stored PIDs
The system SHALL skip a tracked launched process during snapshot collection when `/proc/<pid>` no longer exists.

#### Scenario: Stored PID is missing during snapshot
- **WHEN** a snapshot command inspects a tracked PID whose `/proc/<pid>` path is missing
- **THEN** the system skips that PID for that snapshot without failing solely for that reason

#### Scenario: Stored PID exists during snapshot
- **WHEN** a snapshot command inspects a tracked PID whose `/proc/<pid>` path exists
- **THEN** the system writes `/proc/<pid>/smaps` to a device-local snapshot artifact and records it for later transfer

#### Scenario: App relaunches outside framework control
- **WHEN** an app relaunches with a different PID outside framework control
- **THEN** the system does not rediscover the new PID and continues using only stored PIDs
