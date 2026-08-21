## MODIFIED Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument, reboot controls `--reboot` and `--no-reboot`, and log controls `--logs` and `--no-logs`. Reboot SHALL default to disabled. Log collection SHALL default to enabled. Output SHALL be written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory.

#### Scenario: User requests help
- **WHEN** the user runs `python run.py --help`
- **THEN** the system displays command-line help describing `--flow`, reboot controls, and log controls

#### Scenario: User omits flow argument
- **WHEN** the user runs `python run.py` without `--flow`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides flow input
- **WHEN** the user runs `python run.py --flow flow.json`
- **THEN** the system uses a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` output directory

#### Scenario: User requests reboot
- **WHEN** the user runs `python run.py --flow flow.json --reboot`
- **THEN** benchmark options enable reboot before AppFlows

#### Scenario: User disables reboot explicitly
- **WHEN** the user runs `python run.py --flow flow.json --no-reboot`
- **THEN** benchmark options disable reboot before AppFlows

#### Scenario: User disables logs
- **WHEN** the user runs `python run.py --flow flow.json --no-logs`
- **THEN** benchmark options disable hilog setup, clear, dump, and receive behavior

#### Scenario: User provides contradictory boolean controls
- **WHEN** the user runs `python run.py --flow flow.json` with both members of a boolean control pair such as `--logs --no-logs` or `--reboot --no-reboot`
- **THEN** the system exits with an argument validation error

### Requirement: Benchmark execution accepts injected dependencies
The CLI orchestration layer SHALL accept a `BenchmarkOptions` object and an already-created device-compatible object for a full benchmark run. The lower-level benchmark runner SHALL accept a `BenchmarkOptions` object, an injected `ResultStore`, and an already-created device-compatible object for evidence collection instead of constructing HDC, Device, or result storage internally. `BenchmarkOptions` SHALL contain `flow_path`, `out_dir`, `reboot`, and `logs`; callers MUST provide it explicitly.

#### Scenario: CLI constructs production device and options
- **WHEN** the CLI starts benchmark execution
- **THEN** it loads configuration, constructs the HDC wrapper, constructs the production device, constructs `BenchmarkOptions`, and passes options and device to the full benchmark orchestration function

#### Scenario: Orchestration creates store and reports
- **WHEN** full benchmark orchestration begins with provided options and a provided device object
- **THEN** it creates a `ResultStore`, calls lower-level benchmark execution, and generates reports after lower-level execution succeeds

#### Scenario: Runner uses provided device, store, and options
- **WHEN** benchmark evidence collection begins with provided options, a provided store, and a provided device object
- **THEN** the runner uses the options for flow path, output path, reboot behavior, and log behavior, uses the store for result paths, and uses the device object for all device operations

#### Scenario: Tests pass fake device, explicit options, and store directly
- **WHEN** a runner test executes benchmark flow logic
- **THEN** the test can pass `BenchmarkOptions`, `ResultStore`, and a fake device directly without monkeypatching production device construction or result storage construction

## ADDED Requirements

### Requirement: Benchmark execution prepares device environment before AppFlows
The benchmark runner SHALL prepare the device environment before launching any `AppFlow`. When reboot is enabled, it SHALL reboot the device, wait for HDC availability, and wait for boot completion. Regardless of reboot setting, it SHALL disable screen timeout before AppFlows. When logs are enabled, it SHALL configure hilog before AppFlows.

#### Scenario: Reboot is enabled
- **WHEN** benchmark options have `reboot` equal to `True`
- **THEN** the runner reboots the device, waits for device availability, and waits for boot completion before reading screen bounds or launching AppFlows

#### Scenario: Reboot is disabled
- **WHEN** benchmark options have `reboot` equal to `False`
- **THEN** the runner skips reboot, device availability wait, and boot-complete wait

#### Scenario: Screen timeout is disabled
- **WHEN** benchmark execution prepares the device environment
- **THEN** the runner disables screen timeout before launching AppFlows independent of reboot and log options

#### Scenario: Logs are enabled during preparation
- **WHEN** benchmark options have `logs` equal to `True`
- **THEN** the runner configures hilog before launching AppFlows

### Requirement: Benchmark execution captures per-AppFlow logs
The benchmark runner SHALL collect a hilog artifact for each `AppFlow` when logs are enabled. It SHALL clear hilog immediately before each AppFlow starts and dump hilog immediately after that AppFlow finishes or fails.

#### Scenario: Logs are enabled for successful AppFlow
- **WHEN** an AppFlow runs successfully with log collection enabled
- **THEN** the runner clears hilog before the AppFlow and records a pending log artifact after the AppFlow finishes

#### Scenario: Logs are enabled for failing AppFlow
- **WHEN** an AppFlow fails with log collection enabled
- **THEN** the runner attempts to dump that AppFlow's hilog before propagating the failure

#### Scenario: AppFlow and log collection both fail
- **WHEN** an AppFlow fails and its log collection also fails
- **THEN** the runner preserves the original AppFlow failure and SHOULD include the log collection failure in a combined error when practical

#### Scenario: Logs are disabled
- **WHEN** benchmark options have `logs` equal to `False`
- **THEN** the runner skips hilog configuration, clear, dump, pending log artifact tracking, and log receive behavior
