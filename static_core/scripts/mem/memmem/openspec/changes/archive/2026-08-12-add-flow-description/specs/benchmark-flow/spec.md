## MODIFIED Requirements

### Requirement: Flow JSON defines ordered app flows
The system SHALL load and validate a JSON flow file containing an ordered list of `AppFlow` entries with `label`, `bundle`, `ability`, `terminate`, and `commands` fields. The flow JSON MAY include optional top-level `"$desc"` string metadata that describes the flow for human reviewers and does not affect execution. Commands MUST be validated as an action-discriminated union before execution. App labels and snapshot labels MUST match `^[A-Za-z0-9_-]+$`. Screenshot labels MUST match `^[A-Za-z0-9_-]+$`. App labels MUST be globally unique across the flow. Snapshot labels MUST be globally unique across all `snapshot` commands in the flow. Screenshot labels MUST be globally unique across all `screenshot` commands in the flow. UI coordinate command payloads MUST use integer percent coordinates from `0` to `100`.

#### Scenario: Valid flow loads successfully
- **WHEN** a flow JSON file contains valid `AppFlow` entries with explicit terminate values, unique app labels, unique snapshot labels, and unique screenshot labels
- **THEN** the system loads the flow and preserves the declared order

#### Scenario: Flow description loads successfully
- **WHEN** a flow JSON file contains top-level `"$desc"` with a string value
- **THEN** the system loads the flow and preserves the description metadata without changing benchmark execution

#### Scenario: Flow description must be a string
- **WHEN** a flow JSON file contains top-level `"$desc"` with a non-string value
- **THEN** the system fails validation before performing device actions

#### Scenario: Required field is missing
- **WHEN** a flow JSON file omits a required `AppFlow` field such as `terminate`
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

#### Scenario: Screenshot label contains unsafe characters
- **WHEN** a `screenshot` command payload contains characters outside letters, digits, underscore, and hyphen
- **THEN** the system fails validation before performing device actions

#### Scenario: Duplicate app label is present
- **WHEN** multiple `AppFlow` entries use the same label
- **THEN** the system fails validation before performing device actions

#### Scenario: Duplicate snapshot label is present
- **WHEN** multiple `snapshot` commands anywhere in the flow use the same payload label
- **THEN** the system fails validation before performing device actions

#### Scenario: Duplicate screenshot label is present
- **WHEN** multiple `screenshot` commands anywhere in the flow use the same payload label
- **THEN** the system fails validation before performing device actions

#### Scenario: Snapshot and screenshot labels match
- **WHEN** a `snapshot` command and a `screenshot` command use the same payload label
- **THEN** the system accepts the flow if labels are otherwise valid and unique within their own command type

#### Scenario: UI coordinate is outside normalized range
- **WHEN** a UI command payload contains a percent coordinate less than `0` or greater than `100`
- **THEN** the system fails schema validation before performing device actions
