## MODIFIED Requirements

### Requirement: Recorder CLI supports timeout and manual stop
The recorder CLI SHALL accept an optional named `--timeout` positive integer in seconds and SHALL stop recording when either the timeout elapses after recorder readiness or the user confirms completion through stdin.

#### Scenario: Timeout stops recording
- **WHEN** the user starts recording with a timeout and does not manually stop before the timeout elapses after recorder readiness
- **THEN** the system sends SIGINT to the recorder process and proceeds to read recorded events

#### Scenario: Manual confirmation stops recording
- **WHEN** the user starts recording and types `y` followed by Enter at the completion prompt
- **THEN** the system sends SIGINT to the recorder process and proceeds to read recorded events

#### Scenario: Manual stop wins before timeout
- **WHEN** the user starts recording with a timeout and confirms completion before the timeout elapses after readiness
- **THEN** the system sends SIGINT immediately and does not wait for the timeout

#### Scenario: Invalid timeout fails
- **WHEN** the user provides a timeout that is not a positive integer
- **THEN** the CLI exits with argument validation error before starting device recording
