## MODIFIED Requirements

### Requirement: Device layer supports app lifecycle, PID checks, timestamps, and smaps capture
The system SHALL provide device operations for launching an app, resolving its current PID, checking PID validity, creating remote directories, obtaining device timestamps, capturing smaps to device-local files, and removing remote directories.

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
- **WHEN** the snapshot command needs a timestamp for snapshot metadata
- **THEN** the device layer returns a filename-safe digit-only timestamp produced on the device

#### Scenario: Remote run directory is created
- **WHEN** the runner starts benchmark execution
- **THEN** the device layer creates `/data/local/tmp/memmem-<run_id>/`

#### Scenario: Smaps capture is requested
- **WHEN** the snapshot command captures smaps for a valid PID
- **THEN** the device layer executes `cat /proc/<pid>/smaps > <remote_path>` on the device and reports success or failure to the caller

#### Scenario: Remote run directory is cleaned up
- **WHEN** all pending snapshot artifacts have been received successfully
- **THEN** the device layer removes the remote run directory and reports cleanup failure to the caller
