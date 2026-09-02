## MODIFIED Requirements

### Requirement: Public facade provides flow builders
The public programmatic API SHALL provide builder functions in `lib.py` for constructing valid `Flow`, `AppFlow`, and command models without requiring callers to import from `src.*`. The `flow` builder SHALL accept a list of app models and MAY accept optional description metadata.

#### Scenario: Flow wrapper accepts app models
- **WHEN** a caller invokes `lib.flow()` with a list of app models constructed by `lib.app_flow()`
- **THEN** the wrapper returns a validated `lib.Flow` model containing those app flows

#### Scenario: Flow wrapper accepts description metadata
- **WHEN** a caller invokes `lib.flow()` with `desc` set to a string
- **THEN** the wrapper returns a validated `lib.Flow` model containing that description metadata

#### Scenario: App-flow wrapper requires explicit fields
- **WHEN** a caller invokes `lib.app_flow()`
- **THEN** the caller MUST explicitly provide label, bundle, ability, terminate, and commands

#### Scenario: Command wrappers construct typed commands
- **WHEN** a caller invokes public command wrappers such as `lib.wait()`, `lib.snapshot()`, `lib.screenshot()`, `lib.key()`, `lib.tap()`, or `lib.text()`
- **THEN** each wrapper returns the corresponding validated command model

#### Scenario: Invalid builder input fails validation
- **WHEN** a caller passes invalid labels, coordinates, velocity, or payload values to a public builder
- **THEN** validation fails at construction time
