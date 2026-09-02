## ADDED Requirements

### Requirement: Run-wide hilog stream lifecycle
The system SHALL collect hilog as one run-wide asynchronous stream to a device-local file when log collection is enabled. The stream SHALL start after remote output setup creates the remote logs directory and after startup validation and execution context preparation have succeeded, before AppFlow launch. The stream SHALL stop during benchmark finalization on both success and failure after stream startup.

#### Scenario: Streaming starts after remote log path exists
- **WHEN** log collection is enabled and remote output setup has created the remote logs directory
- **THEN** the system starts a run-wide hilog stream to the fixed remote path `logs/hilog.log`

#### Scenario: Streaming starts after startup validation actions
- **WHEN** log collection is enabled and the remote hilog stream starts
- **THEN** the stream starts after device health verification and execution context preparation, before AppFlow launch

#### Scenario: Streaming stops after successful AppFlows
- **WHEN** log collection is enabled and all AppFlows complete successfully
- **THEN** the system stops the run-wide hilog stream during finalization and preserves the remote stream artifact for pending receive

#### Scenario: Streaming stops after benchmark failure
- **WHEN** log collection is enabled and benchmark execution fails at any point after stream startup
- **THEN** the system still attempts to stop the run-wide hilog stream and preserve any collected remote output

#### Scenario: Streaming stop targets descendants
- **WHEN** the run-wide hilog stream is stopped
- **THEN** the system attempts to stop both the stream-owning child process and its HDC subprocess descendants before receiving the log artifact

#### Scenario: Streaming is disabled
- **WHEN** benchmark options have `logs` equal to `False`
- **THEN** the system does not configure hilog, start a hilog stream, stop a hilog stream, or record a pending log artifact

### Requirement: HDC hilog stream assumptions are explicit
The system SHALL treat `hilog -x` as a buffered non-blocking dump and SHALL use `hilog` without `-x` for continuous collection.

#### Scenario: Buffered dump is avoided
- **WHEN** log collection is enabled
- **THEN** the system does not rely on `hilog -x` for benchmark runtime coverage

#### Scenario: Continuous command is used
- **WHEN** the run-wide log stream is started
- **THEN** the system invokes an HDC shell command equivalent to `hilog > <remote_path>` without `-x`
