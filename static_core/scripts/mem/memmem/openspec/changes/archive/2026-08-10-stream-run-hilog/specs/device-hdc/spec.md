## MODIFIED Requirements

### Requirement: Device layer supports benchmark environment controls
The system SHALL provide device operations for waking the device, disabling screen timeout, configuring hilog collection, and blocking run-wide hilog streaming to a device-local file.

#### Scenario: Device is woken
- **WHEN** the runner requests device wakeup
- **THEN** the device layer executes `power-shell wakeup` through HDC shell and reports failure to the caller when the command fails

#### Scenario: Screen timeout is disabled
- **WHEN** the runner requests screen timeout disabling
- **THEN** the device layer executes `power-shell timeout -o 60000000` through HDC shell and reports failure to the caller when the command fails

#### Scenario: Hilog is configured
- **WHEN** the runner configures hilog collection
- **THEN** the device layer executes `hilog -Q pidoff` and `hilog -p off` through HDC shell and reports failure when either command fails

#### Scenario: Hilog stream is run
- **WHEN** the runner requests hilog streaming to a remote path
- **THEN** the device layer executes blocking command `hilog > <remote_path>` through HDC shell and reports command failure to the caller

#### Scenario: Hilog stream path is required
- **WHEN** the runner requests hilog streaming
- **THEN** the caller MUST provide a device-local remote path and the device layer does not stream hilog into host memory
