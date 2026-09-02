## MODIFIED Requirements

### Requirement: Test doubles support benchmark execution
The test suite SHALL provide a reusable fake device object in `test/mock/device.py` with configurable screen bounds, process state, invalid bundles, remote directories, remote files, health observations, failure flags for device operations used by benchmark execution, and fake generic child process handles for run-wide hilog streams. The fake device SHALL support reboot, boot readiness, wakeup, screen-timeout disabling, hilog configuration, hilog stream start/stop through generic child processes, app termination, and screenshot capture.

#### Scenario: Valid app launch creates process state
- **WHEN** the fake device launches an app with a valid bundle
- **THEN** it records process state for that bundle and returns a stable PID for later resolution

#### Scenario: Repeated app launch reuses process state
- **WHEN** a bundle that already has process state is launched again
- **THEN** the fake device keeps the existing PID for that bundle

#### Scenario: Invalid app launch does not create process state
- **WHEN** the fake device launches a bundle configured as invalid
- **THEN** the fake device does not add process state for that bundle and PID resolution fails

#### Scenario: Fake device terminates app
- **WHEN** benchmark execution requests app termination for a known bundle
- **THEN** the fake device removes process state for that bundle

#### Scenario: Fake device termination fails
- **WHEN** benchmark execution requests app termination and termination is configured to fail
- **THEN** the fake device raises a termination failure

#### Scenario: Fake device starts hilog child process
- **WHEN** test code requests run-wide hilog streaming to a remote path on a fake device instance and hilog streaming is not configured to fail
- **THEN** that fake device instance returns a fake generic child process handle and stores fake log content in its remote files map at that path

#### Scenario: Fake hilog child process stops
- **WHEN** benchmark finalization stops a hilog child process returned by the fake device
- **THEN** the process exits without requiring multiprocessing shared memory or Unix process-group signaling

#### Scenario: Fake device hilog stream fails
- **WHEN** benchmark execution requests run-wide hilog streaming to a remote path and hilog streaming is configured to fail
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

#### Scenario: Hilog stream process lifecycle is portable
- **WHEN** a test validates run-wide hilog stream process lifecycle behavior
- **THEN** the test does not require `os.setsid`, `os.killpg`, Unix signals, or multiprocessing process-group cleanup
