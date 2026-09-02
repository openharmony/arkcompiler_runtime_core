## MODIFIED Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument, an optional `--out-dir` argument, an optional `--repeats` argument, reboot controls `--reboot` and `--no-reboot`, hilog controls `--hilog` and `--no-hilog`, and memmem application logging controls `--memmem-log-level` and `--memmem-log-file`. Reboot SHALL default to disabled. Hilog collection SHALL default to enabled and SHALL mean one run-wide device hilog artifact. Memmem log level SHALL default to `err`. Memmem log file SHALL default to empty. Repeats SHALL default to `1` and SHALL be a positive integer; a non-positive value SHALL be rejected at argument parsing. When `--out-dir` is omitted, output SHALL be written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory. When `--out-dir` is provided, output SHALL be written to the provided directory, and an already existing output directory SHALL be rejected. The CLI SHALL act as an adapter that reads the JSON flow file, validates it into the flow schema, constructs the production device, and delegates execution to the public programmatic API.

#### Scenario: User requests help
- **WHEN** the user runs `python run.py --help`
- **THEN** the system displays command-line help describing `--flow`, `--out-dir`, `--repeats`, reboot controls, hilog controls, and memmem logging controls

#### Scenario: User omits flow argument
- **WHEN** the user runs `python run.py` without `--flow`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides flow input
- **WHEN** the user runs `python run.py --flow flow.json`
- **THEN** the system uses a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` output directory and benchmark options disable reboot, enable hilog collection, set repeats to `1`, set memmem log level to `err`, and set memmem log file to empty by default

#### Scenario: User provides out dir explicitly
- **WHEN** the user runs `python run.py --flow flow.json --out-dir results/bench1`
- **THEN** the system writes all run output under the provided `results/bench1` directory

#### Scenario: User provides an existing out dir
- **WHEN** the user runs `python run.py --flow flow.json --out-dir results/bench1` and `results/bench1` already exists
- **THEN** the run fails before device actions with an error naming the output directory

#### Scenario: User provides repeats explicitly
- **WHEN** the user runs `python run.py --flow flow.json --repeats 3`
- **THEN** benchmark options set repeats to `3` and the flow runs once per iteration into iteration output directories

#### Scenario: User provides a non-positive repeats value
- **WHEN** the user runs `python run.py --flow flow.json --repeats 0` or `--repeats -2`
- **THEN** the system exits with an argument validation error

#### Scenario: User requests reboot explicitly
- **WHEN** the user runs `python run.py --flow flow.json --reboot`
- **THEN** benchmark options enable reboot before each iteration's AppFlows

#### Scenario: User disables reboot explicitly
- **WHEN** the user runs `python run.py --flow flow.json --no-reboot`
- **THEN** benchmark options disable reboot before AppFlows

#### Scenario: User enables hilog explicitly
- **WHEN** the user runs `python run.py --flow flow.json --hilog`
- **THEN** benchmark options enable run-wide device hilog collection

#### Scenario: User disables hilog explicitly
- **WHEN** the user runs `python run.py --flow flow.json --no-hilog`
- **THEN** benchmark options disable hilog setup, stream start, stream stop, pending hilog artifact tracking, and hilog receive behavior

#### Scenario: User configures memmem log level
- **WHEN** the user runs `python run.py --flow flow.json --memmem-log-level info`
- **THEN** benchmark options set memmem application log level to `info`

#### Scenario: User configures memmem log file
- **WHEN** the user runs `python run.py --flow flow.json --memmem-log-file memmem.log`
- **THEN** benchmark options route emitted memmem application logs to `memmem.log`

#### Scenario: User provides invalid memmem log level
- **WHEN** the user runs `python run.py --flow flow.json --memmem-log-level debug`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides removed log controls
- **WHEN** the user runs `python run.py --flow flow.json` with `--logs` or `--no-logs`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides contradictory boolean controls
- **WHEN** the user runs `python run.py --flow flow.json` with both members of a boolean control pair such as `--hilog --no-hilog` or `--reboot --no-reboot`
- **THEN** the system exits with an argument validation error

#### Scenario: CLI delegates to public API
- **WHEN** CLI argument parsing, configuration loading, device construction, and flow validation succeed
- **THEN** the CLI calls the public programmatic run API with the validated flow, device, resolved output directory, reboot option, hilog option, repeats option, memmem log level, and memmem log file

## ADDED Requirements

### Requirement: CLI path arguments resolve against the invocation working directory
The system SHALL resolve the `--flow` path, the `--out-dir` path, the `--memmem-log-file` path, and the `HDC_PATH` configuration value against the working directory of the CLI invocation before any later working directory change.

#### Scenario: Relative path arguments are invariant to the run working directory
- **WHEN** the user runs `python run.py --flow flow.json --out-dir out` and benchmark execution later changes the process working directory
- **THEN** the flow file, output directory, and memmem log file still resolve to the paths given at invocation time

### Requirement: Benchmark execution manages its working directory
The system SHALL change the working directory to the local output directory immediately after local output initialization and SHALL restore the original working directory after the run, including when the run fails. Local paths supplied to device file-transfer operations SHALL be relative to the run working directory.

#### Scenario: Run executes from the output directory
- **WHEN** benchmark execution starts with a validated flow and output directory
- **THEN** the system executes device interactions from a working directory equal to the output directory after local output initialization

#### Scenario: Working directory is restored after the run
- **WHEN** benchmark execution finishes or fails
- **THEN** the process working directory equals the working directory captured at benchmark entry

#### Scenario: File transfers use run-relative local paths
- **WHEN** a pending device-local artifact is received
- **THEN** the local path supplied to the file-transfer operation is relative to the run working directory