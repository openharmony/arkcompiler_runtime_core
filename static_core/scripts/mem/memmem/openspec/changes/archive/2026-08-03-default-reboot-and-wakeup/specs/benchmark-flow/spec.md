## MODIFIED Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument, reboot controls `--reboot` and `--no-reboot`, and log controls `--logs` and `--no-logs`. Reboot SHALL default to enabled. Log collection SHALL default to enabled. Output SHALL be written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory.

#### Scenario: User requests help
- **WHEN** the user runs `python run.py --help`
- **THEN** the system displays command-line help describing `--flow`, reboot controls, and log controls

#### Scenario: User omits flow argument
- **WHEN** the user runs `python run.py` without `--flow`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides flow input
- **WHEN** the user runs `python run.py --flow flow.json`
- **THEN** the system uses a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` output directory and benchmark options enable reboot before AppFlows

#### Scenario: User requests reboot explicitly
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

### Requirement: Benchmark execution prepares device environment before AppFlows
The benchmark runner SHALL prepare the device environment before launching any `AppFlow`. When reboot is enabled, it SHALL reboot the device, wait for HDC availability, wait for boot completion, wake the device, perform an upward directional fling, and send Back. Regardless of reboot setting, it SHALL disable screen timeout before AppFlows. When logs are enabled, it SHALL configure hilog before AppFlows.

#### Scenario: Reboot is enabled
- **WHEN** benchmark options have `reboot` equal to `True`
- **THEN** the runner reboots the device, waits for device availability, waits for boot completion, performs the post-reboot wake routine, and then reads screen bounds before launching AppFlows

#### Scenario: Reboot is disabled
- **WHEN** benchmark options have `reboot` equal to `False`
- **THEN** the runner skips reboot, device availability wait, boot-complete wait, and post-reboot wake routine

#### Scenario: Post-reboot wake routine runs
- **WHEN** reboot preparation reaches boot completion
- **THEN** the runner wakes the device, performs an upward directional fling, and sends Back before disabling screen timeout

#### Scenario: Screen timeout is disabled
- **WHEN** benchmark execution prepares the device environment
- **THEN** the runner disables screen timeout before launching AppFlows independent of reboot and log options

#### Scenario: Logs are enabled during preparation
- **WHEN** benchmark options have `logs` equal to `True`
- **THEN** the runner configures hilog before launching AppFlows
