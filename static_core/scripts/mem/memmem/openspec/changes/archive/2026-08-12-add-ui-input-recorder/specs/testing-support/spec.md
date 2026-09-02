## MODIFIED Requirements

### Requirement: Programmatic interface tests cover public facade behavior
The test suite SHALL verify the public `lib.py` facade, CLI validation behavior, flow revalidation boundary, canonical flow evidence, flow description metadata behavior, recorder parsing/conversion behavior, exported public type names, and typed user-script compatibility without requiring users to import documented APIs from `src.*`.

#### Scenario: Public builders construct valid models
- **WHEN** tests construct a flow using `lib.flow()`, `lib.app_flow()`, and command wrappers
- **THEN** the resulting flow can be passed to `lib.run()` and executed with a fake device

#### Scenario: Public flow builder accepts description
- **WHEN** tests construct a flow using `lib.flow()` with `desc` set to a string
- **THEN** the resulting flow preserves the description metadata

#### Scenario: Builder validation failures are surfaced
- **WHEN** tests invoke public builder wrappers with invalid labels or invalid command payload values
- **THEN** the wrapper raises validation failure at construction time

#### Scenario: Public run revalidates mutated flow
- **WHEN** tests mutate a valid flow to break model-level invariants before passing it to `lib.run()`
- **THEN** `lib.run()` fails validation before performing device actions

#### Scenario: Public run returns no result object
- **WHEN** tests call `lib.run()` successfully
- **THEN** the call returns `None` and output is available under the caller-provided output directory

#### Scenario: Canonical flow output is tested
- **WHEN** tests run either CLI or programmatic execution
- **THEN** output `flow.json` is asserted as canonical validated flow JSON rather than a byte-for-byte input copy

#### Scenario: Canonical flow output preserves description
- **WHEN** tests run CLI or programmatic execution with a flow description
- **THEN** output `flow.json` contains the canonical `"$desc"` metadata

#### Scenario: Recorder parses and converts sample record
- **WHEN** tests pass a sample `uitest uiRecord` file containing window bounds and supported pointer/key JSON records to the recorder conversion layer
- **THEN** the generated model is a valid `Flow` containing normalized UI commands

#### Scenario: Recorder output filename is timestamped
- **WHEN** tests run recorder output serialization with a fixed clock
- **THEN** the output file name matches `flow-YYYYMMDD-HHMMSS-MILLISECONDS.json`

#### Scenario: Recorder preserves identity ordering
- **WHEN** tests convert a record containing interleaved bundle/ability identity segments, including empty identity values
- **THEN** generated app flows preserve segment order, use empty strings for empty identity fields, set `terminate: false`, and preserve input order within each segment

#### Scenario: Recorder skips unsupported events
- **WHEN** tests convert a record containing unsupported event types mixed with supported events
- **THEN** unsupported rows are reported as warnings and supported commands remain in the generated flow

#### Scenario: Old app helper is not public
- **WHEN** tests inspect the public facade exports
- **THEN** `app_flow` is exported and `app` is not available as a public helper

#### Scenario: Public type names are exported
- **WHEN** tests inspect the public facade exports
- **THEN** flow, app-flow, command, HDC, and device type names are exported while payload struct and internal runner option types are not exported

#### Scenario: User script type-checks with public types
- **WHEN** tests run mypy against a fixture that imports and annotates with public `lib.py` type names
- **THEN** mypy succeeds without importing documented APIs from `src.*`
