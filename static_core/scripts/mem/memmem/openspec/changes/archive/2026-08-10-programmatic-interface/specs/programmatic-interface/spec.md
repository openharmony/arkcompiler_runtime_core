## ADDED Requirements

### Requirement: Programmatic API runs validated flows
The system SHALL provide a public `lib.py` programmatic API that runs benchmark flows constructed in Python without requiring callers to write JSON flow files. The API SHALL accept flow content explicitly, accept a device handle, accept keyword-only runtime options, revalidate the flow before execution, run evidence collection, generate reports, return `None` on success, and report failure by raising an exception.

#### Scenario: Programmatic run succeeds
- **WHEN** a caller passes a valid programmatically constructed flow, a device handle, and explicit `out_dir` to `lib.run()`
- **THEN** the system executes the benchmark, generates reports, writes output under the requested directory, and returns `None`

#### Scenario: Runtime options are keyword-only
- **WHEN** a caller invokes `lib.run()`
- **THEN** the caller MUST provide `out_dir` as a keyword argument and MAY provide `reboot` and `logs` keyword arguments

#### Scenario: Runtime option defaults match CLI
- **WHEN** a caller omits `reboot` and `logs` from `lib.run()`
- **THEN** the system uses `reboot=False` and `logs=True`

#### Scenario: Programmatic flow is revalidated
- **WHEN** a caller passes a flow object whose invariants were broken after construction
- **THEN** `lib.run()` fails validation before performing device actions

#### Scenario: Programmatic run failure raises
- **WHEN** benchmark execution or report generation fails during `lib.run()`
- **THEN** the failure is reported by raising an exception rather than returning a status object

### Requirement: Programmatic API constructs flows with thin wrappers
The system SHALL provide public `lib.py` wrapper functions for constructing benchmark flows, app flows, and command models. These wrappers SHALL construct the existing validated Pydantic schema models immediately and SHALL NOT introduce hidden defaults unless the underlying schema model defines them.

#### Scenario: Flow wrapper accepts app models
- **WHEN** a caller invokes `lib.flow()` with a list of app models constructed by `lib.app()`
- **THEN** the system returns a validated flow model

#### Scenario: App wrapper requires explicit fields
- **WHEN** a caller invokes `lib.app()`
- **THEN** the caller MUST explicitly provide label, bundle, ability, terminate, and commands

#### Scenario: Command wrappers validate immediately
- **WHEN** a caller invokes a command wrapper with invalid labels, coordinates, velocity, step length, direction, key, or text
- **THEN** the wrapper raises validation failure during construction

#### Scenario: Repetitive scenarios can be generated with Python
- **WHEN** a caller uses Python loops or helper functions to assemble repeated command sequences
- **THEN** the wrapper-produced models can be combined into a valid flow for `lib.run()`

### Requirement: Programmatic API creates device handles through public helpers
The system SHALL provide public `lib.py` helpers for constructing HDC and device handles without requiring documented user scripts to import `src.*` modules.

#### Scenario: HDC helper accepts path-like input
- **WHEN** a caller invokes `lib.get_hdc()` with a string or `pathlib.Path` HDC executable path
- **THEN** the system returns an HDC handle usable by the public device helper

#### Scenario: Device helper wraps HDC handle
- **WHEN** a caller invokes `lib.get_device()` with an HDC handle returned by `lib.get_hdc()`
- **THEN** the system returns a device handle usable by `lib.run()`

#### Scenario: Public script avoids src imports
- **WHEN** a user follows documented programmatic API examples
- **THEN** the script imports from `lib.py` and does not need to import from `src.*`
