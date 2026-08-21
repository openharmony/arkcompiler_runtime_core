## MODIFIED Requirements

### Requirement: Shared fake HDC supports command logging and configured results
The test suite SHALL provide a reusable fake HDC object in `test/mock/hdc.py` that records every `run` and `shell` invocation and returns configured `HdcResult` values without executing external commands. The fake HDC SHALL support the same timeout parameter as the production HDC wrapper and MAY be configured with `timeout_after` to return `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")` when a provided timeout is too small.

#### Scenario: Default HDC result is returned
- **WHEN** a test calls `run` or `shell` with no matching configured response
- **THEN** the fake HDC returns its configured default `HdcResult`

#### Scenario: Raw run arguments are recorded
- **WHEN** a test calls `run` with HDC arguments
- **THEN** the fake HDC records exactly those arguments without adding a `run` marker

#### Scenario: Exact HDC response is returned
- **WHEN** a test calls `run` or `shell` with arguments matching a configured response key
- **THEN** the fake HDC returns the matching `HdcResult` and records the call

#### Scenario: Prefix HDC response is returned
- **WHEN** a test calls `run` or `shell` with arguments matching a configured response prefix and no exact response matches
- **THEN** the fake HDC returns the matching prefix `HdcResult` and records the call

#### Scenario: Fake HDC timeout is simulated
- **WHEN** a fake HDC is configured with `timeout_after` and a command is called with a timeout smaller than that threshold
- **THEN** the fake HDC returns `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")`

### Requirement: Shared fake device models benchmark-relevant device state
The test suite SHALL provide a reusable fake device object in `test/mock/device.py` with configurable screen bounds, process state, invalid bundles, remote directories, remote files, and failure flags for device operations used by benchmark execution, including reboot, boot readiness, screen-timeout disabling, and hilog operations.

#### Scenario: Valid app launch creates process state
- **WHEN** a valid bundle is launched for the first time
- **THEN** the fake device stores a bundle-to-PID mapping that can be resolved later

#### Scenario: Repeated app launch reuses process state
- **WHEN** a bundle that already has process state is launched again
- **THEN** the fake device keeps the existing PID for that bundle

#### Scenario: Invalid app launch does not create process state
- **WHEN** a bundle configured as invalid is launched
- **THEN** the fake device does not add process state for that bundle and PID resolution fails

#### Scenario: Screen bounds are returned from configured state
- **WHEN** benchmark execution asks for screen bounds and no screen error is configured
- **THEN** the fake device returns its configured `ScreenBounds`

#### Scenario: Screen bounds failure is configured
- **WHEN** benchmark execution asks for screen bounds and a screen error is configured
- **THEN** the fake device raises that error

#### Scenario: Remote directory is created
- **WHEN** benchmark execution creates a remote directory and directory creation is not configured to fail
- **THEN** the fake device records that path in its remote directory set

#### Scenario: Smaps capture requires parent directory
- **WHEN** benchmark execution captures smaps to a remote path whose parent directory is not recorded
- **THEN** the fake device reports smaps capture failure

#### Scenario: Smaps capture writes remote file
- **WHEN** benchmark execution captures smaps for an existing PID to a path whose parent directory exists
- **THEN** the fake device stores smaps content in its remote files map

#### Scenario: Remote file receive writes local file
- **WHEN** benchmark execution receives a remote file that exists in fake device state
- **THEN** the fake device writes the file content to the requested local path and returns a successful `HdcResult`

#### Scenario: Remote directory removal clears subtree
- **WHEN** benchmark execution removes a remote directory and removal is not configured to fail
- **THEN** the fake device removes matching remote directories and files under that path

#### Scenario: Fake device reboots
- **WHEN** benchmark execution requests reboot and reboot is not configured to fail
- **THEN** the fake device records reboot state

#### Scenario: Fake device dumps hilog
- **WHEN** benchmark execution requests hilog dump to a remote path and dump is not configured to fail
- **THEN** the fake device stores fake log content in its remote files map at that path
