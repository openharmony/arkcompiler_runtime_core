## ADDED Requirements

### Requirement: HDC wrapper executes generic commands
The system SHALL provide an HDC wrapper that executes `<HDC_PATH> <args...>` and returns return code, stdout, and stderr without imposing success policy.

#### Scenario: Caller runs generic HDC command
- **WHEN** a caller invokes `Hdc.run('list', 'targets')`
- **THEN** the wrapper executes `<HDC_PATH> list targets` and returns an `HdcResult`

#### Scenario: HDC command exits non-zero
- **WHEN** an HDC command returns a non-zero exit code
- **THEN** the wrapper returns the non-zero return code, stdout, and stderr to the caller

### Requirement: HDC wrapper executes shell commands
The system SHALL provide `Hdc.shell` for executing `<HDC_PATH> shell <args...>` and returning return code, stdout, and stderr.

#### Scenario: Caller runs shell command
- **WHEN** a caller invokes `Hdc.shell('pidof', 'com.example.app')`
- **THEN** the wrapper executes `<HDC_PATH> shell pidof com.example.app` and returns an `HdcResult`

### Requirement: Device layer supports app lifecycle, PID checks, and smaps capture
The system SHALL provide device operations for launching an app, resolving its PID, checking PID validity, creating remote directories, obtaining device timestamps, capturing smaps to device-local files, and removing remote directories.

#### Scenario: App launch is requested
- **WHEN** the runner starts an `AppFlow`
- **THEN** the device layer launches the requested bundle and ability using HDC commands based on the legacy memmem implementation

#### Scenario: PID resolution is requested
- **WHEN** the runner needs the launched app PID
- **THEN** the device layer resolves and returns an integer PID not present in the excluded PID set or reports failure to the caller

#### Scenario: PID validity is checked
- **WHEN** the snapshot command checks a stored PID
- **THEN** the device layer reports whether `/proc/<pid>` exists on the device

#### Scenario: Device timestamp is requested
- **WHEN** the runner or snapshot command needs a timestamp
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

### Requirement: Device layer supports key events
The system SHALL provide a key event operation that accepts command-specific string payload parameters and sends the corresponding key action through HDC.

#### Scenario: Key command delegates to device
- **WHEN** a valid `key` command is executed
- **THEN** the device layer sends the corresponding key event to the device

### Requirement: Device layer supports file transfer utilities
The system SHALL provide file send and receive operations using HDC file transfer commands for device-side snapshot artifacts and future device-side workflows.

#### Scenario: File send is requested
- **WHEN** a caller invokes `send_file(local_path, remote_path)`
- **THEN** the device layer executes `hdc file send <local_path> <remote_path>`

#### Scenario: File receive is requested
- **WHEN** a caller invokes `recv_file(remote_path, local_path)`
- **THEN** the device layer creates the local parent directory and executes `hdc file recv <remote_path> <local_path>`
