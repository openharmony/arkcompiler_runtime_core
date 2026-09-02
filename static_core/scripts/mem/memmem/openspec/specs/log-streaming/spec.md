# log-streaming Specification

## Purpose

Defines run-wide device-local hilog stream lifecycle behavior.

## Requirements

### Requirement: Run-wide hilog stream lifecycle
The system SHALL collect hilog as one run-wide asynchronous stream to a device-local file when hilog collection is enabled. The stream SHALL start after remote output setup creates the remote hilog directory and after startup validation and execution context preparation have succeeded, before AppFlow launch. The stream SHALL remain active until benchmark finalization requests shutdown and SHALL stop during finalization on both success and failure after stream startup. A host-side HDC process found already exited before requested shutdown SHALL fail the benchmark regardless of its host exit code, because that code does not prove complete remote hilog coverage. Stream start and stop MUST use a generic child process handle rather than Unix-only process group operations.

#### Scenario: Streaming starts after remote hilog path exists
- **WHEN** hilog collection is enabled and remote output setup has created the remote hilog directory
- **THEN** the system starts a run-wide hilog stream to the fixed remote path `hilog/hilog.log`

#### Scenario: Streaming starts after startup validation actions
- **WHEN** hilog collection is enabled and the remote hilog stream starts
- **THEN** the stream starts after device health verification and execution context preparation, before AppFlow launch

#### Scenario: Streaming stops after successful AppFlows
- **WHEN** hilog collection is enabled and all AppFlows complete successfully
- **THEN** the system stops the run-wide hilog stream during finalization and preserves the remote stream artifact for pending receive

#### Scenario: Streaming stops after benchmark failure
- **WHEN** hilog collection is enabled and benchmark execution fails at any point after stream startup
- **THEN** the system still attempts to stop the run-wide hilog stream and preserve any collected remote output

#### Scenario: Hilog stream exits before requested shutdown
- **WHEN** the host-side HDC process for the run-wide hilog stream has already exited before finalization requests shutdown, including with host exit code `0`
- **THEN** the system fails the benchmark and reports the host exit code

#### Scenario: Partial hilog is preserved after early exit
- **WHEN** an early hilog stream exit is detected after the stream produced a partial device-local artifact
- **THEN** finalization still receives the partial `hilog.log`, writes metadata, cleans the remote run directory, and then reports the benchmark failure

#### Scenario: Streaming stop uses portable process APIs
- **WHEN** the run-wide hilog stream is stopped
- **THEN** the system stops the generic host-side HDC child process using Python built-in cross-platform process APIs without relying on `os.setsid`, `os.killpg`, or Unix signals

#### Scenario: Streaming is disabled
- **WHEN** benchmark options have `hilog` equal to `False`
- **THEN** the system does not configure hilog, start a hilog stream, stop a hilog stream, or record a pending hilog artifact

### Requirement: HDC hilog stream assumptions are explicit
The system SHALL treat `hilog -x` as a buffered non-blocking dump and SHALL use `hilog` without `-x` for continuous collection.

#### Scenario: Buffered dump is avoided
- **WHEN** hilog collection is enabled
- **THEN** the system does not rely on `hilog -x` for benchmark runtime coverage

#### Scenario: Continuous command is used
- **WHEN** the run-wide hilog stream is started
- **THEN** the system invokes an HDC shell command equivalent to `hilog > <remote_path>` without `-x`
