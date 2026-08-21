## MODIFIED Requirements

### Requirement: Programmatic API constructs flows with thin wrappers
The system SHALL provide public `lib.py` wrapper functions for constructing benchmark flows, app flows, and command models. These wrappers SHALL construct the existing validated Pydantic schema models immediately and SHALL NOT introduce hidden defaults unless the underlying schema model defines them.

#### Scenario: Flow wrapper accepts app models
- **WHEN** a caller invokes `lib.flow()` with a list of app models constructed by `lib.app_flow()`
- **THEN** the system returns a validated flow model

#### Scenario: App-flow wrapper requires explicit fields
- **WHEN** a caller invokes `lib.app_flow()`
- **THEN** the caller MUST explicitly provide label, bundle, ability, terminate, and commands

#### Scenario: Command wrappers validate immediately
- **WHEN** a caller invokes a command wrapper with invalid labels, coordinates, velocity, step length, direction, key, or text
- **THEN** the wrapper raises validation failure during construction

#### Scenario: Repetitive scenarios can be generated with Python
- **WHEN** a caller uses Python loops or helper functions to assemble repeated command sequences
- **THEN** the wrapper-produced models can be combined into a valid flow for `lib.run()`

#### Scenario: Old app wrapper is not public
- **WHEN** a caller imports documented public helpers from `lib.py`
- **THEN** `app_flow` is available and `app` is not part of the public facade
