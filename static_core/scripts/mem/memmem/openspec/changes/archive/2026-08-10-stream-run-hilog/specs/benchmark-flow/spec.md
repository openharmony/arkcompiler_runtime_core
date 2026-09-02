## MODIFIED Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument, reboot controls `--reboot` and `--no-reboot`, and log controls `--logs` and `--no-logs`. Reboot SHALL default to disabled. Log collection SHALL default to enabled and SHALL mean one run-wide hilog artifact. Output SHALL be written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory.

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

### Requirement: Benchmark execution prepares device environment before AppFlows
The benchmark runner SHALL prepare the device environment before launching any `AppFlow`. When reboot is enabled, it SHALL reboot the device, wait for HDC availability, wait for boot completion, wake the device, perform an upward directional fling, and send Back. Regardless of reboot setting, it SHALL disable screen timeout before AppFlows. When logs are enabled, it SHALL configure hilog before health verification and remote setup. It SHALL verify that device health is acceptable before remote output setup. It SHALL create remote output and prepare execution context before starting any enabled run-wide hilog stream. After startup validation and any enabled stream startup completes, it SHALL launch AppFlows.

#### Scenario: Reboot is enabled
- **WHEN** benchmark options have `reboot` equal to `True`
- **THEN** the runner reboots the device, waits for device availability, waits for boot completion, performs the post-reboot wake routine, configures enabled hilog, verifies device health, creates remote output, reads screen bounds, and then starts enabled logging before launching AppFlows

#### Scenario: Reboot is disabled
- **WHEN** benchmark options have `reboot` equal to `False`
- **THEN** the runner skips reboot, device availability wait, boot-complete wait, and post-reboot wake routine while still configuring enabled hilog, verifying device health, creating remote output, reading screen bounds, and starting enabled logging before launching AppFlows

#### Scenario: Post-reboot wake routine runs
- **WHEN** reboot preparation reaches boot completion
- **THEN** the runner wakes the device, performs an upward directional fling, and sends Back before disabling screen timeout

#### Scenario: Device health is acceptable
- **WHEN** benchmark execution prepares the device environment and device health reports battery capacity at least 30% and every readable nonzero thermal zone at or below 80000 millidegrees Celsius
- **THEN** the runner continues preparation and may launch AppFlows

#### Scenario: Battery capacity is too low
- **WHEN** benchmark execution prepares the device environment and device health reports battery capacity below 30%
- **THEN** the runner rejects benchmark startup before starting any enabled hilog stream, reading screen bounds, or launching AppFlows

#### Scenario: Thermal zone is too hot
- **WHEN** benchmark execution prepares the device environment and any readable nonzero thermal zone reports a temperature above 80000 millidegrees Celsius
- **THEN** the runner rejects benchmark startup before starting any enabled hilog stream, reading screen bounds, or launching AppFlows

#### Scenario: Screen timeout is disabled
- **WHEN** benchmark execution prepares the device environment
- **THEN** the runner disables screen timeout before device health verification and before launching AppFlows independent of reboot and log options

#### Scenario: Logs are enabled during preparation
- **WHEN** benchmark options have `logs` equal to `True`
- **THEN** the runner configures hilog before remote output setup and starts the run-wide hilog stream after remote output setup, device health verification, and execution context preparation

### Requirement: Benchmark execution captures run-wide logs
The benchmark runner SHALL collect one run-wide hilog artifact when logs are enabled. It SHALL NOT clear hilog before each AppFlow and SHALL NOT dump hilog after each AppFlow. Requested AppFlow termination SHALL run after AppFlow command execution without waiting for any per-AppFlow log dump.

#### Scenario: Logs are enabled for successful benchmark
- **WHEN** all AppFlows run successfully with log collection enabled
- **THEN** the runner records one pending log artifact and receives it as `logs/hilog.log` after stopping the hilog stream

#### Scenario: Logs are enabled for failing benchmark
- **WHEN** benchmark execution fails after the run-wide hilog stream has started
- **THEN** the runner attempts to stop the hilog stream before propagating the failure and still attempts to receive pending artifacts

#### Scenario: AppFlow and log stream stop both fail
- **WHEN** an AppFlow fails and stopping the hilog stream also fails
- **THEN** the runner preserves the original AppFlow failure and SHOULD include the log stream stop failure in a combined error when practical

#### Scenario: Logs are disabled
- **WHEN** benchmark options have `logs` equal to `False`
- **THEN** the runner skips hilog configuration, stream start, stream stop, log artifact tracking, and log receive behavior

#### Scenario: Termination does not wait for per-AppFlow dump
- **WHEN** an AppFlow has `terminate` equal to `True` and log collection is enabled
- **THEN** the runner requests bundle termination after AppFlow command execution without performing a per-AppFlow hilog dump
