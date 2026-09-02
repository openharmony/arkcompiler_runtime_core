## MODIFIED Requirements

### Requirement: Device layer supports benchmark environment controls
The system SHALL provide device operations for waking the device, disabling screen timeout, configuring hilog collection, and starting run-wide hilog streaming to a device-local file through a generic child process handle. The child process handle MUST allow the caller to stop the host-side HDC stream process using Python built-in cross-platform process APIs.

#### Scenario: Device is woken
- **WHEN** the runner requests device wakeup
- **THEN** the device layer executes `power-shell wakeup` through HDC shell and reports failure to the caller when the command fails

#### Scenario: Screen timeout is disabled
- **WHEN** the runner requests screen timeout disabling
- **THEN** the device layer executes `power-shell timeout -o 60000000` through HDC shell and reports failure to the caller when the command fails

#### Scenario: Hilog is configured
- **WHEN** the runner configures hilog collection
- **THEN** the device layer executes `hilog -Q pidoff` and `hilog -p off` through HDC shell and reports failure when either command fails

#### Scenario: Hilog stream is started
- **WHEN** the runner requests hilog streaming to a remote path
- **THEN** the device layer starts command `hilog > <remote_path>` through HDC shell and returns a generic child process handle without streaming hilog into host memory

#### Scenario: Hilog stream is stopped
- **WHEN** the caller stops a generic hilog child process handle
- **THEN** the system terminates the direct host-side HDC stream process, waits for bounded shutdown, and kills the process if it remains alive using Python built-in cross-platform process APIs

#### Scenario: Hilog stream path is required
- **WHEN** the runner requests hilog streaming
- **THEN** the caller MUST provide a device-local remote path and the device layer does not stream hilog into host memory
