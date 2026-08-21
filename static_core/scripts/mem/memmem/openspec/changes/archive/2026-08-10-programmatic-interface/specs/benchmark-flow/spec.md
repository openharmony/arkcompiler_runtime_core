## MODIFIED Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument, reboot controls `--reboot` and `--no-reboot`, and log controls `--logs` and `--no-logs`. Reboot SHALL default to disabled. Log collection SHALL default to enabled and SHALL mean one run-wide hilog artifact. Output SHALL be written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory. The CLI SHALL act as an adapter that reads the JSON flow file, validates it into the flow schema, constructs the production device, and delegates execution to the public programmatic API.

#### Scenario: User requests help
- **WHEN** the user runs `python run.py --help`
- **THEN** the system displays command-line help describing `--flow`, reboot controls, and run-wide log controls

#### Scenario: User omits flow argument
- **WHEN** the user runs `python run.py` without `--flow`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides flow input
- **WHEN** the user runs `python run.py --flow flow.json`
- **THEN** the system uses a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` output directory and benchmark options disable reboot while enabling log collection by default

#### Scenario: User requests reboot explicitly
- **WHEN** the user runs `python run.py --flow flow.json --reboot`
- **THEN** benchmark options enable reboot before AppFlows

#### Scenario: User disables reboot explicitly
- **WHEN** the user runs `python run.py --flow flow.json --no-reboot`
- **THEN** benchmark options disable reboot before AppFlows

#### Scenario: User enables logs explicitly
- **WHEN** the user runs `python run.py --flow flow.json --logs`
- **THEN** benchmark options enable run-wide hilog collection

#### Scenario: User disables logs explicitly
- **WHEN** the user runs `python run.py --flow flow.json --no-logs`
- **THEN** benchmark options disable hilog setup, stream start, stream stop, pending log artifact tracking, and log receive behavior

#### Scenario: User provides contradictory boolean controls
- **WHEN** the user runs `python run.py --flow flow.json` with both members of a boolean control pair such as `--logs --no-logs` or `--reboot --no-reboot`
- **THEN** the system exits with an argument validation error

#### Scenario: CLI delegates to public API
- **WHEN** CLI argument parsing, configuration loading, device construction, and flow validation succeed
- **THEN** the CLI calls the public programmatic run API with the validated flow, device, timestamped output directory, reboot option, and logs option

### Requirement: Benchmark execution accepts injected dependencies
The CLI orchestration layer SHALL accept a validated flow object, runtime options, and an already-created device-compatible object for a full benchmark run. The lower-level benchmark runner SHALL accept a validated flow object, runtime options, an injected `ResultStore`, and an already-created device-compatible object for evidence collection instead of constructing HDC, Device, result storage, or flow content internally. Runtime options SHALL contain `out_dir`, `reboot`, and `logs`; callers MUST provide flow content separately from runtime options.

#### Scenario: CLI constructs production device and options
- **WHEN** the CLI starts benchmark execution
- **THEN** it loads configuration, constructs the HDC wrapper, constructs the production device, validates the JSON flow into a flow object, and passes the flow, runtime options, and device to the public programmatic API

#### Scenario: Public API creates store and reports
- **WHEN** public programmatic execution begins with a provided flow, runtime option arguments, and a provided device object
- **THEN** it revalidates the flow, creates internal runtime options, creates a `ResultStore`, calls lower-level benchmark execution, and generates reports after lower-level execution succeeds

#### Scenario: Runner uses provided flow, device, store, and options
- **WHEN** benchmark evidence collection begins with a provided flow, runtime options, a provided store, and a provided device object
- **THEN** the runner uses the provided flow for AppFlow content, uses options for output path, reboot behavior, and log behavior, uses the store for result paths, and uses the device object for all device operations

#### Scenario: Tests pass fake device, explicit flow, options, and store directly
- **WHEN** a runner test executes benchmark flow logic
- **THEN** the test can pass a validated flow object, internal runtime options, `ResultStore`, and a fake device directly without monkeypatching production device construction or result storage construction
