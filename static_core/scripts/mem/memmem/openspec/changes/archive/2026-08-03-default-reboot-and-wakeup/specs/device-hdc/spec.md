## MODIFIED Requirements

### Requirement: Device layer supports benchmark environment controls
The system SHALL provide device operations for waking the device, disabling screen timeout, configuring hilog collection, clearing hilog, and dumping hilog to a device-local file.

#### Scenario: Device is woken
- **WHEN** the runner requests device wakeup
- **THEN** the device layer executes `power-shell wakeup` through HDC shell and reports failure to the caller when the command fails

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
