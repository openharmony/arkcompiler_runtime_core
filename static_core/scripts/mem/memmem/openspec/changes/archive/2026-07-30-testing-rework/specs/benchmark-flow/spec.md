## ADDED Requirements

### Requirement: Benchmark execution accepts injected device dependency
The benchmark runner SHALL accept an already-created device-compatible object for benchmark execution instead of constructing HDC and Device instances internally.

#### Scenario: CLI constructs production device
- **WHEN** the CLI starts benchmark execution
- **THEN** it loads configuration, constructs the HDC wrapper, constructs the production device, and passes that device to benchmark execution

#### Scenario: Runner uses provided device
- **WHEN** benchmark execution begins with a provided device object
- **THEN** the runner uses that object for screen bounds, remote directory management, app lifecycle, PID resolution, snapshot capture, artifact receive, and cleanup

#### Scenario: Tests pass fake device directly
- **WHEN** a runner test executes benchmark flow logic
- **THEN** the test can pass a fake device directly without monkeypatching production device construction
