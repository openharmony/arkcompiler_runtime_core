## MODIFIED Requirements

### Requirement: Programmatic interface tests cover public facade behavior
The test suite SHALL verify the public `lib.py` facade, CLI validation behavior, flow revalidation boundary, and canonical flow evidence without requiring users to import documented APIs from `src.*`.

#### Scenario: Public builders construct valid models
- **WHEN** tests construct a flow using `lib.flow()`, `lib.app_flow()`, and command wrappers
- **THEN** the resulting flow can be passed to `lib.run()` and executed with a fake device

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

#### Scenario: Old app helper is not public
- **WHEN** tests inspect the public facade exports
- **THEN** `app_flow` is exported and `app` is not available as a public helper
