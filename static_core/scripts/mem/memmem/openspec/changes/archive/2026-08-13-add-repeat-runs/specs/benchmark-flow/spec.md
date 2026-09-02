## MODIFIED Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument, an optional `--repeats` argument, reboot controls `--reboot` and `--no-reboot`, and log controls `--logs` and `--no-logs`. Reboot SHALL default to disabled. Log collection SHALL default to enabled and SHALL mean one run-wide hilog artifact. Repeats SHALL default to `1` and SHALL be a positive integer; a non-positive value SHALL be rejected at argument parsing. Output SHALL be written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory. The CLI SHALL act as an adapter that reads the JSON flow file, validates it into the flow schema, constructs the production device, and delegates execution to the public programmatic API.

#### Scenario: User requests help
- **WHEN** the user runs `python run.py --help`
- **THEN** the system displays command-line help describing `--flow`, `--repeats`, reboot controls, and run-wide log controls

#### Scenario: User omits flow argument
- **WHEN** the user runs `python run.py` without `--flow`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides flow input
- **WHEN** the user runs `python run.py --flow flow.json`
- **THEN** the system uses a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` output directory and benchmark options disable reboot, enable log collection, and set repeats to `1` by default

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
- **THEN** the CLI calls the public programmatic run API with the validated flow, device, timestamped output directory, reboot option, logs option, and repeats option

## ADDED Requirements

### Requirement: Repeated runs produce iteration output directories
The system SHALL execute a benchmark flow a configured number of times when repeats is greater than `1`. For repeat index `i` from `0` to `repeats - 1`, the system SHALL store the full run output in the `iteration_{i}` directory under the run output directory, with each iteration directory having the same structure as a single run. When repeats is `1`, the system SHALL store output directly in the run output directory without an iteration subdirectory. Each iteration SHALL generate its own per-iteration reports. After all iterations complete, the system SHALL generate averaged results in a `summary` directory under the run output directory. Reboot, when enabled, SHALL apply before each iteration's flow execution.

#### Scenario: Repeats equals one
- **WHEN** the user runs a benchmark with repeats equal to `1`
- **THEN** the system stores output directly in the run output directory and does not create any `iteration_{i}` directory or `summary` directory

#### Scenario: Repeats is greater than one
- **WHEN** the user runs a benchmark with repeats greater than `1`
- **THEN** the system creates `iteration_0` through `iteration_{repeats-1}` directories under the run output directory, each containing the full per-run output and per-iteration reports

#### Scenario: Iteration failure aborts the run
- **WHEN** one iteration of a repeated benchmark fails
- **THEN** the system fails the benchmark run and does not produce a `summary` directory, while already-written iteration directories remain on disk

#### Scenario: Reboot applies per iteration
- **WHEN** the user runs a repeated benchmark with reboot enabled
- **THEN** the system reboots the device before each iteration's flow execution