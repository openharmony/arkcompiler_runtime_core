## MODIFIED Requirements

### Requirement: HDC wrapper executes generic commands
The system SHALL provide an HDC wrapper that executes `<HDC_PATH> <args...>` and returns return code, stdout, and stderr without imposing success policy. The wrapper SHALL accept a command timeout where `timeout=-1` means no timeout. If command execution times out, the wrapper SHALL return `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")`.

#### Scenario: Caller runs generic HDC command
- **WHEN** a caller invokes `Hdc.run('list', 'targets')`
- **THEN** the wrapper executes `<HDC_PATH> list targets` and returns an `HdcResult`

#### Scenario: HDC command exits non-zero
- **WHEN** an HDC command returns a non-zero exit code
- **THEN** the wrapper returns the non-zero return code, stdout, and stderr to the caller

#### Scenario: HDC command times out
- **WHEN** an HDC command exceeds the non-negative timeout provided by the caller
- **THEN** the wrapper returns `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")`

#### Scenario: HDC timeout is disabled
- **WHEN** a caller invokes `Hdc.run` with `timeout=-1`
- **THEN** the wrapper executes the command without a subprocess timeout

### Requirement: HDC wrapper executes shell commands
The system SHALL provide `Hdc.shell` for executing `<HDC_PATH> shell <args...>` and returning return code, stdout, and stderr. The shell wrapper SHALL accept the same timeout semantics as `Hdc.run`.

#### Scenario: Caller runs shell command
- **WHEN** a caller invokes `Hdc.shell('pidof', 'com.example.app')`
- **THEN** the wrapper executes `<HDC_PATH> shell pidof com.example.app` and returns an `HdcResult`

#### Scenario: Caller runs shell command with timeout
- **WHEN** a caller invokes `Hdc.shell` with a non-negative timeout
- **THEN** the wrapper applies that timeout to the underlying HDC shell command

## ADDED Requirements

### Requirement: Device layer supports reboot and boot readiness
The system SHALL provide device operations for rebooting the device, waiting for HDC availability, and waiting for OpenHarmony boot completion.

#### Scenario: Device reboot is requested
- **WHEN** the runner requests device reboot
- **THEN** the device layer executes `hdc target boot` and reports failure to the caller when the command fails

#### Scenario: Device availability wait is requested
- **WHEN** the runner waits for device availability
- **THEN** the device layer executes `hdc wait` with the provided timeout and reports failure to the caller when the command fails or times out

#### Scenario: Boot completion wait succeeds
- **WHEN** the runner waits for boot completion and `param get bootevent.boot.completed` returns `true` before timeout
- **THEN** the device layer reports boot completion success

#### Scenario: Boot completion wait times out
- **WHEN** `param get bootevent.boot.completed` does not return `true` before timeout
- **THEN** the device layer reports boot completion failure to the caller

### Requirement: Device layer supports benchmark environment controls
The system SHALL provide device operations for disabling screen timeout, configuring hilog collection, clearing hilog, and dumping hilog to a device-local file.

#### Scenario: Screen timeout is disabled
- **WHEN** the runner requests screen timeout disabling
- **THEN** the device layer executes `power-shell timeout -o 60000000` through HDC shell and reports failure to the caller when the command fails

#### Scenario: Hilog is configured
- **WHEN** the runner configures hilog collection
- **THEN** the device layer executes `hilog -Q pidoff` and `hilog -p off` through HDC shell and reports failure when either command fails

#### Scenario: Hilog is cleared
- **WHEN** the runner clears hilog for an AppFlow
- **THEN** the device layer executes `hilog -r` through HDC shell and reports failure to the caller when the command fails

#### Scenario: Hilog is dumped
- **WHEN** the runner dumps hilog to a remote log path
- **THEN** the device layer executes `hilog -x > <remote_path>` through HDC shell and reports failure to the caller when the command fails
