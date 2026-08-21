## MODIFIED Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument, an optional `--repeats` argument, reboot controls `--reboot` and `--no-reboot`, hilog controls `--hilog` and `--no-hilog`, and memmem application logging controls `--memmem-log-level` and `--memmem-log-file`. Reboot SHALL default to disabled. Hilog collection SHALL default to enabled and SHALL mean one run-wide device hilog artifact. Memmem log level SHALL default to `err`. Memmem log file SHALL default to empty. Repeats SHALL default to `1` and SHALL be a positive integer; a non-positive value SHALL be rejected at argument parsing. Output SHALL be written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory. The CLI SHALL act as an adapter that reads the JSON flow file, validates it into the flow schema, constructs the production device, and delegates execution to the public programmatic API.

#### Scenario: User requests help
- **WHEN** the user runs `python run.py --help`
- **THEN** the system displays command-line help describing `--flow`, `--repeats`, reboot controls, hilog controls, and memmem logging controls

#### Scenario: User omits flow argument
- **WHEN** the user runs `python run.py` without `--flow`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides flow input
- **WHEN** the user runs `python run.py --flow flow.json`
- **THEN** the system uses a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` output directory and benchmark options disable reboot, enable hilog collection, set repeats to `1`, set memmem log level to `err`, and set memmem log file to empty by default

#### Scenario: User provides repeats explicitly
- **WHEN** the user runs `python run.py --flow flow.json --repeats 3`
- **THEN** benchmark options set repeats to `3` and the flow runs once per iteration into iteration output directories

#### Scenario: User provides a non-positive repeats value
- **WHEN** the user runs `python run.py --flow flow.json --repeats 0` or `--repeats -2`
- **THEN** the system exits with an argument validation error

#### Scenario: User requests reboot explicitly
- **WHEN** the user runs `python run.py --flow flow.json --reboot`
- **THEN** benchmark options enable reboot before each iteration's AppFlows

#### Scenario: User disables reboot explicitly
- **WHEN** the user runs `python run.py --flow flow.json --no-reboot`
- **THEN** benchmark options disable reboot before AppFlows

#### Scenario: User enables hilog explicitly
- **WHEN** the user runs `python run.py --flow flow.json --hilog`
- **THEN** benchmark options enable run-wide device hilog collection

#### Scenario: User disables hilog explicitly
- **WHEN** the user runs `python run.py --flow flow.json --no-hilog`
- **THEN** benchmark options disable hilog setup, stream start, stream stop, pending hilog artifact tracking, and hilog receive behavior

#### Scenario: User configures memmem log level
- **WHEN** the user runs `python run.py --flow flow.json --memmem-log-level info`
- **THEN** benchmark options set memmem application log level to `info`

#### Scenario: User configures memmem log file
- **WHEN** the user runs `python run.py --flow flow.json --memmem-log-file memmem.log`
- **THEN** benchmark options route emitted memmem application logs to `memmem.log`

#### Scenario: User provides invalid memmem log level
- **WHEN** the user runs `python run.py --flow flow.json --memmem-log-level debug`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides removed log controls
- **WHEN** the user runs `python run.py --flow flow.json` with `--logs` or `--no-logs`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides contradictory boolean controls
- **WHEN** the user runs `python run.py --flow flow.json` with both members of a boolean control pair such as `--hilog --no-hilog` or `--reboot --no-reboot`
- **THEN** the system exits with an argument validation error

#### Scenario: CLI delegates to public API
- **WHEN** CLI argument parsing, configuration loading, device construction, and flow validation succeed
- **THEN** the CLI calls the public programmatic run API with the validated flow, device, timestamped output directory, reboot option, hilog option, repeats option, memmem log level, and memmem log file

### Requirement: Benchmark execution accepts injected dependencies
The CLI orchestration layer SHALL accept a validated flow object, runtime options, and an already-created device-compatible object for a full benchmark run. The lower-level benchmark runner SHALL accept a validated flow object, runtime options, an injected `ResultStore`, and an already-created device-compatible object for evidence collection instead of constructing HDC, Device, result storage, flow content, or logging configuration internally. Runtime options SHALL contain `out_dir`, `reboot`, `hilog`, and `repeats`; callers MUST provide flow content separately from runtime options. Memmem logging SHALL be read from the module-level logging configuration rather than passed through runner function arguments.

#### Scenario: CLI constructs production device and options
- **WHEN** the CLI starts benchmark execution
- **THEN** it loads configuration, constructs the HDC wrapper, constructs the production device, validates the JSON flow into a flow object, configures memmem logging, and passes the flow, runtime options, and device to the public programmatic API

#### Scenario: Public API creates store and reports
- **WHEN** public programmatic execution begins with a provided flow, runtime option arguments, a provided device object, and previously configured memmem logging
- **THEN** it revalidates the flow, creates internal runtime options, creates a `ResultStore`, calls lower-level benchmark execution, and generates per-run or averaged reports after lower-level execution succeeds

#### Scenario: Runner uses provided flow, device, store, options, and configured logger
- **WHEN** benchmark evidence collection begins with a provided flow, runtime options, a provided store, a provided device object, and configured memmem logging
- **THEN** the runner uses the provided flow for AppFlow content, uses options for output path, reboot behavior, hilog behavior, and repeat count, uses the store for result paths, uses the device object for all device operations, and uses the configured singleton logger for memmem runner messages

#### Scenario: Tests pass fake device, explicit flow, options, and store directly
- **WHEN** a runner test executes benchmark flow logic
- **THEN** the test can pass a validated flow object, internal runtime options, `ResultStore`, and fake device directly without monkeypatching production device construction or result storage construction
