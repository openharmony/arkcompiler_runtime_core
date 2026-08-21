## MODIFIED Requirements

### Requirement: Shared fake device models benchmark-relevant device state
The test suite SHALL provide a reusable fake device object in `test/mock/device.py` with configurable screen bounds, process state, invalid bundles, remote directories, remote files, health observations, and failure flags for device operations used by benchmark execution, including reboot, boot readiness, wakeup, screen-timeout disabling, hilog configuration, blocking hilog streaming, app termination, and screenshot capture.

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

#### Scenario: Screenshot capture requires parent directory
- **WHEN** benchmark execution captures a screenshot to a remote path whose parent directory is not recorded
- **THEN** the fake device reports screenshot capture failure

#### Scenario: Screenshot capture writes remote file
- **WHEN** benchmark execution captures a screenshot to a path whose parent directory exists
- **THEN** the fake device stores fake PNG content in its remote files map

#### Scenario: Remote file receive writes local file
- **WHEN** benchmark execution receives a remote file that exists in fake device state
- **THEN** the fake device writes the file content to the requested local path and returns a successful `HdcResult`

#### Scenario: Remote directory removal clears subtree
- **WHEN** benchmark execution removes a remote directory and removal is not configured to fail
- **THEN** the fake device removes matching remote directories and files under that path

#### Scenario: Fake device reboots
- **WHEN** benchmark execution requests reboot and reboot is not configured to fail
- **THEN** the fake device records reboot state

#### Scenario: Fake device wakes up
- **WHEN** benchmark execution requests wakeup and wakeup is not configured to fail
- **THEN** the fake device records wakeup state

#### Scenario: Fake device reports health
- **WHEN** benchmark execution requests device health and health reporting is not configured to fail
- **THEN** the fake device returns its configured battery capacity and thermal zone observations

#### Scenario: Fake device health fails
- **WHEN** benchmark execution requests device health and health reporting is configured to fail
- **THEN** the fake device raises a device health failure

#### Scenario: Fake device terminates app
- **WHEN** benchmark execution requests app termination and termination is not configured to fail
- **THEN** the fake device removes process state for that bundle

#### Scenario: Fake device termination fails
- **WHEN** benchmark execution requests app termination and termination is configured to fail
- **THEN** the fake device raises a termination failure

#### Scenario: Fake device runs hilog stream directly
- **WHEN** test code requests blocking hilog streaming to a remote path on a fake device instance and hilog streaming is not configured to fail
- **THEN** that fake device instance stores fake log content in its remote files map at that path

#### Scenario: Fake runner receive observes hilog artifact
- **WHEN** benchmark execution receives `hilog.log` from a fake device after a multiprocessing hilog child was started
- **THEN** the fake device may synthesize fake log content during receive because child-process memory is isolated from the parent fake instance

#### Scenario: Fake device hilog stream fails
- **WHEN** benchmark execution requests blocking hilog streaming to a remote path and hilog streaming is configured to fail
- **THEN** the fake device raises a hilog streaming failure

### Requirement: Device command translation tests use fake HDC logging
Tests that assert concrete HDC command arguments SHALL use the real device layer with fake HDC call logging rather than fake device method logging.

#### Scenario: UI command translation is asserted
- **WHEN** a test validates the HDC command emitted for a UI input operation
- **THEN** the test uses real `Device` behavior and inspects `FakeHdc.calls`

#### Scenario: Termination command translation is asserted
- **WHEN** a test validates the HDC command emitted for app termination
- **THEN** the test uses real `Device` behavior and inspects `FakeHdc.calls`

#### Scenario: Screenshot command translation is asserted
- **WHEN** a test validates the HDC command emitted for screenshot capture
- **THEN** the test uses real `Device` behavior and inspects `FakeHdc.calls`

#### Scenario: Hilog stream command translation is asserted
- **WHEN** a test validates the HDC command emitted for run-wide hilog streaming
- **THEN** the test uses real `Device` behavior and inspects `FakeHdc.calls`
