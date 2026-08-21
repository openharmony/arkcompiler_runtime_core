## MODIFIED Requirements

### Requirement: Device layer supports app lifecycle, PID checks, timestamps, and smaps capture
The system SHALL provide device operations for launching an app, resolving its current PID, checking PID validity, creating remote directories, obtaining device timestamps, capturing smaps to device-local files, capturing screenshots to device-local files, terminating an app bundle, and removing remote directories.

#### Scenario: App launch is requested
- **WHEN** the runner starts an `AppFlow`
- **THEN** the device layer launches the requested bundle and ability using HDC shell commands based on the legacy memmem implementation

#### Scenario: PID resolution returns one PID
- **WHEN** the runner needs the launched app PID and `pidof <bundle>` returns exactly one parseable PID
- **THEN** the device layer returns that PID even when it was already tracked for another app label

#### Scenario: PID resolution returns no PID
- **WHEN** the runner needs the launched app PID and `pidof <bundle>` returns no parseable PID
- **THEN** the device layer reports PID resolution failure to the caller

#### Scenario: PID resolution returns multiple PIDs
- **WHEN** the runner needs the launched app PID and `pidof <bundle>` returns more than one parseable PID
- **THEN** the device layer reports PID resolution failure to the caller without choosing a PID

#### Scenario: PID validity is checked
- **WHEN** the snapshot command checks a stored PID
- **THEN** the device layer reports whether `/proc/<pid>` exists on the device

#### Scenario: Device timestamp is requested
- **WHEN** a snapshot or screenshot command needs a timestamp for metadata
- **THEN** the device layer returns a filename-safe digit-only timestamp produced on the device

#### Scenario: Remote run directory is created
- **WHEN** the runner starts benchmark execution
- **THEN** the device layer creates the requested remote run directory under `/data/local/tmp`

#### Scenario: Smaps capture is requested
- **WHEN** the snapshot command captures smaps for a valid PID
- **THEN** the device layer executes `cat /proc/<pid>/smaps > <remote_path>` on the device and reports success or failure to the caller

#### Scenario: Screenshot capture is requested
- **WHEN** the screenshot command captures the current screen
- **THEN** the device layer executes `uitest screenCap -p <remote_path>` on the device and reports success or failure to the caller

#### Scenario: App termination is requested
- **WHEN** the runner requests termination for a bundle
- **THEN** the device layer executes `aa force-stop <bundle>` through HDC shell and reports success only when command output contains `force stop process successfully`

#### Scenario: App termination command reports an error in stdout
- **WHEN** `aa force-stop <bundle>` returns output containing an error message or no success marker
- **THEN** the device layer reports app termination failure to the caller even when the HDC return code is zero

#### Scenario: Remote run directory is cleaned up
- **WHEN** pending artifacts have been received successfully
- **THEN** the device layer removes the remote run directory and reports cleanup failure to the caller
