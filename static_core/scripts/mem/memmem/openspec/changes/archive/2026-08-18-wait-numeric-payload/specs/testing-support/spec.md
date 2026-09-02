## ADDED Requirements

### Requirement: Schema regression coverage for wait payload numerics and flat text payload
The test suite SHALL verify that `wait` payload validation accepts non-negative integer and float values, rejects negative and non-numeric values, and that the flat `text` action rejects the legacy struct payload form.

#### Scenario: Wait payload accepts non-negative numeric values
- **WHEN** tests validate `wait` commands with non-negative integer and float payloads
- **THEN** the commands load and validate successfully

#### Scenario: Wait payload rejects negative and non-numeric values
- **WHEN** tests validate `wait` commands with negative or non-numeric payloads
- **THEN** schema validation fails

#### Scenario: Flat text payload rejects legacy struct form
- **WHEN** tests validate a `text` command whose payload is a struct such as `{"text": "..."}` instead of a flat string
- **THEN** schema validation fails