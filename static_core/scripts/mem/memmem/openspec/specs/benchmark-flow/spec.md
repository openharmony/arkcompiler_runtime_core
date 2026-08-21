# benchmark-flow Specification

## Purpose
Defines the benchmark CLI, flow schema, app launch behavior, command semantics, and PID handling for benchmark execution.

## Requirements

### Requirement: CLI accepts benchmark flow input
The system SHALL provide a single user-facing CLI through `run.py` that accepts a required `--flow` argument, an optional `--out-dir` argument, an optional `--repeats` argument, reboot controls `--reboot` and `--no-reboot`, hilog controls `--hilog` and `--no-hilog`, and memmem application logging controls `--memmem-log-level` and `--memmem-log-file`. Reboot SHALL default to disabled. Hilog collection SHALL default to enabled and SHALL mean one run-wide device hilog artifact. Memmem log level SHALL default to `err`. Memmem log file SHALL default to unset (`None`); an empty-string value SHALL be rejected and fail the run before benchmark execution proceeds. Repeats SHALL default to `1` and SHALL be a positive integer; a non-positive value SHALL be rejected at argument parsing. When `--out-dir` is omitted, output SHALL be written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory. When `--out-dir` is provided, output SHALL be written to the provided directory, and an already existing output directory SHALL be rejected. The CLI SHALL act as an adapter that reads the JSON flow file, validates it into the flow schema, constructs the production device, and delegates execution to the public programmatic API.

#### Scenario: User requests help
- **WHEN** the user runs `python run.py --help`
- **THEN** the system displays command-line help describing `--flow`, `--out-dir`, `--repeats`, reboot controls, hilog controls, and memmem logging controls

#### Scenario: User omits flow argument
- **WHEN** the user runs `python run.py` without `--flow`
- **THEN** the system exits with an argument validation error

#### Scenario: User provides flow input
- **WHEN** the user runs `python run.py --flow flow.json`
- **THEN** the system uses a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` output directory and benchmark options disable reboot, enable hilog collection, set repeats to `1`, set memmem log level to `err`, and leave memmem log file unset by default

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

### Requirement: Benchmark execution accepts injected dependencies
The CLI orchestration layer SHALL accept a validated flow object, runtime options, and an already-created device-compatible object for a full benchmark run. The lower-level benchmark runner SHALL accept a validated flow object, runtime options, an injected `ResultStore`, and an already-created device-compatible object for evidence collection instead of constructing HDC, Device, result storage, flow content, or logging configuration internally. Runtime options SHALL contain `out_dir`, `reboot`, `hilog`, and `repeats`; callers MUST provide flow content separately from runtime options. Memmem logging SHALL be read from the module-level logging configuration rather than passed through runner function arguments.

#### Scenario: CLI constructs production device and options
- **WHEN** the CLI starts benchmark execution
- **THEN** it loads configuration, constructs the HDC wrapper, constructs the production device, validates the JSON flow into a flow object, configures memmem logging, and passes the flow, runtime options, and device to the public programmatic API

#### Scenario: Public API creates store and reports
- **WHEN** public programmatic execution begins with a provided flow, runtime option arguments, a provided device object, and previously configured memmem logging
- **THEN** it revalidates the flow, creates internal runtime options, creates a `ResultStore`, calls lower-level benchmark execution, and generates per-run or averaged reports after lower-level execution succeeds

#### Scenario: Runner uses provided flow, device, store, options, and configured logger
- **WHEN** benchmark evidence collection begins with a provided flow, runtime options, a provided store, a provided device object, and configured memmem logging
- **THEN** the runner uses the provided flow for AppFlow content, uses options for output path, reboot behavior, hilog behavior, and repeat count, uses the store for result paths, uses the device object for all device operations, and uses the configured singleton logger for memmem runner messages

#### Scenario: Tests pass fake device, explicit flow, options, and store directly
- **WHEN** a runner test executes benchmark flow logic
- **THEN** the test can pass a validated flow object, internal runtime options, `ResultStore`, and fake device directly without monkeypatching production device construction or result storage construction

### Requirement: Repeated runs produce iteration output directories
The system SHALL execute a benchmark flow a configured number of times when repeats is greater than `1`. The system SHALL atomically reserve the top-level run output directory before the first iteration and SHALL reject an existing top-level directory even when `iteration_0` is absent. For repeat index `i` from `0` to `repeats - 1`, the system SHALL store the full run output in the `iteration_{i}` directory under the run output directory, with each iteration directory having the same structure as a single run. When repeats is `1`, the system SHALL store output directly in the run output directory without an iteration subdirectory. Each iteration SHALL generate its own per-iteration reports. After all iterations complete, the system SHALL generate averaged results in a `summary` directory under the run output directory. Reboot, when enabled, SHALL apply before each iteration's flow execution.

#### Scenario: Repeats equals one
- **WHEN** the user runs a benchmark with repeats equal to `1`
- **THEN** the system stores output directly in the run output directory and does not create any `iteration_{i}` directory or `summary` directory

#### Scenario: Repeats is greater than one
- **WHEN** the user runs a benchmark with repeats greater than `1`
- **THEN** the system creates `iteration_0` through `iteration_{repeats-1}` directories under the run output directory, each containing the full per-run output and per-iteration reports

#### Scenario: Existing repeated-run output is rejected
- **WHEN** the user requests repeats greater than `1` with an existing top-level output directory
- **THEN** the system fails before the first iteration and before device actions without modifying the directory

#### Scenario: Iteration failure aborts the run
- **WHEN** one iteration of a repeated benchmark fails
- **THEN** the system fails the benchmark run and does not produce a `summary` directory, while already-written iteration directories remain on disk

#### Scenario: Reboot applies per iteration
- **WHEN** the user runs a repeated benchmark with reboot enabled
- **THEN** the system reboots the device before each iteration's flow execution

### Requirement: Flow JSON defines ordered app flows
The system SHALL load and validate a JSON flow file as an unprocessed flow containing an ordered list of `AppFlow` entries with `label`, `bundle`, `ability`, `terminate`, and `commands` fields, and SHALL preprocess the unprocessed flow into a canonical `Flow` through the public preprocessing API before execution. Unprocessed command entries MAY be concrete canonical commands or repeat macro entries. Repeat macro entries MUST be expanded before canonical `Flow` validation and before execution. Canonical `Flow` validation MUST enforce that app labels and snapshot labels match `^[A-Za-z0-9_-]+$`, that screenshot labels match `^[A-Za-z0-9_-]+$`, that app labels are globally unique across the flow, that snapshot labels are globally unique across all expanded `snapshot` commands, that screenshot labels are globally unique across all expanded `screenshot` commands, and that UI coordinate command payloads use integer percent coordinates from `0` to `100`. The flow JSON MAY include optional top-level `"$desc"` string metadata that describes the flow for human reviewers and does not affect execution.

#### Scenario: Valid flow loads successfully
- **WHEN** a flow JSON file contains valid `AppFlow` entries with explicit terminate values, unique app labels, unique snapshot labels, and unique screenshot labels
- **THEN** the system preprocesses the flow and preserves the declared order

#### Scenario: Generated recorder flow loads successfully
- **WHEN** a flow JSON file generated by the UI input recorder contains generated app labels, recorded bundle/ability metadata, and converted UI commands
- **THEN** the system loads the flow as a normal canonical benchmark flow

#### Scenario: Flow description loads successfully
- **WHEN** a flow JSON file contains top-level `"$desc"` with a string value
- **THEN** the system loads the flow and preserves the description metadata without changing benchmark execution

#### Scenario: Flow description must be a string
- **WHEN** a flow JSON file contains top-level `"$desc"` with a non-string value
- **THEN** the system fails validation before performing device actions

#### Scenario: Required field is missing
- **WHEN** a flow JSON file omits a required `AppFlow` field such as `terminate`
- **THEN** the system fails validation before performing device actions

#### Scenario: Invalid JSON is provided
- **WHEN** the flow path points to malformed JSON
- **THEN** the system fails validation before performing device actions

#### Scenario: App label contains unsafe characters
- **WHEN** an unprocessed `AppFlow` label contains characters outside letters, digits, underscore, and hyphen
- **THEN** the system fails canonical validation during preprocessing before performing device actions

#### Scenario: Snapshot label contains unsafe characters
- **WHEN** an expanded `snapshot` command payload contains characters outside letters, digits, underscore, and hyphen
- **THEN** the system fails canonical validation before performing device actions

#### Scenario: Screenshot label contains unsafe characters
- **WHEN** an expanded `screenshot` command payload contains characters outside letters, digits, underscore, and hyphen
- **THEN** the system fails canonical validation before performing device actions

#### Scenario: Duplicate app label is present
- **WHEN** multiple `AppFlow` entries use the same label
- **THEN** the system fails canonical validation during preprocessing before performing device actions

#### Scenario: Duplicate snapshot label is present
- **WHEN** multiple expanded `snapshot` commands anywhere in the flow use the same payload label
- **THEN** the system fails canonical validation before performing device actions

#### Scenario: Duplicate screenshot label is present
- **WHEN** multiple expanded `screenshot` commands anywhere in the flow use the same payload label
- **THEN** the system fails canonical validation before performing device actions

#### Scenario: Snapshot and screenshot labels match
- **WHEN** a `snapshot` command and a `screenshot` command use the same payload label after any macro expansion
- **THEN** the system accepts the flow if labels are otherwise valid and unique within their own command type

#### Scenario: UI coordinate is outside normalized range
- **WHEN** a UI command payload contains a percent coordinate less than `0` or greater than `100`
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Top-level placeholder label is rejected at unprocessed validation
- **WHEN** a non-macro snapshot or screenshot command at the unprocessed top level uses a label containing `{` or `}` that no macro can substitute
- **THEN** the system fails unprocessed flow validation before expansion and before performing device actions

#### Scenario: Placeholder label inside macro body is accepted pre-expansion
- **WHEN** a repeat macro body snapshot or screenshot command uses a label containing `{<iter_var>}`
- **THEN** the unprocessed flow validates, and the label passes canonical validation only if the substituted result matches the label regex

### Requirement: Flow JSON supports repeat macro authoring
The system SHALL accept repeat macro entries inside unprocessed `AppFlow.commands` arrays. A repeat macro entry MUST have `macro` equal to `repeat` and a payload containing `iter_var`, `n_iter`, and `commands`. `iter_var` MUST match `^[A-Za-z_][A-Za-z0-9_]*$`. `n_iter` MUST be a strict non-negative integer with no upper bound. `commands` MUST be a list of concrete unprocessed commands and MUST NOT contain nested macros. `n_iter` of zero and an empty `commands` list SHALL be accepted and SHALL expand to no commands. The system SHALL expand repeat macros into concrete commands before canonical `Flow` validation and before performing device actions.

#### Scenario: Repeat macro expands in command order
- **WHEN** a flow contains a repeat macro with `n_iter` equal to `2` and two concrete commands in its payload
- **THEN** the system expands the macro into four concrete commands preserving the body order for each iteration before execution

#### Scenario: Loop variable substitutes string values
- **WHEN** a repeat macro has `iter_var` equal to `i` and a repeated command whose payload is a string contains `{i}`
- **THEN** the system replaces `{i}` with the 0-based iteration number in the canonical range `[0, n_iter-1]` in each expanded command, and string values inside payload structs such as `input_text.text` are not substituted

#### Scenario: N_iter of zero expands to nothing
- **WHEN** a repeat macro has `n_iter` equal to `0`
- **THEN** the macro expands to no commands without error

#### Scenario: Empty macro body expands to nothing
- **WHEN** a repeat macro has an empty `commands` list
- **THEN** the macro expands to no commands without error

#### Scenario: String substitution does not coerce numeric fields
- **WHEN** a repeat macro body command carries a string loop variable token in a numeric payload field such as `wait.payload`
- **THEN** macro body validation rejects the unprocessed flow before expansion because body commands reuse canonical strict payload types and strings are never coerced into numbers

#### Scenario: Macro body without loop variable can fail duplicate label validation
- **WHEN** a repeat macro repeats a `snapshot` command whose payload label does not include the loop variable and expansion creates duplicate snapshot labels
- **THEN** canonical `Flow` validation rejects the expanded flow before performing device actions

#### Scenario: Nested macros are rejected
- **WHEN** a repeat macro payload contains a repeat macro in its `commands` list
- **THEN** the system fails validation before performing device actions

#### Scenario: Invalid repeat macro payload fails validation
- **WHEN** a repeat macro has invalid `iter_var`, invalid `n_iter`, or missing payload fields
- **THEN** the system fails validation before performing device actions

### Requirement: Text command payload is a non-empty string
The system SHALL accept `text` commands only with a non-empty string payload and SHALL delegate the string to the device layer. A `text` command payload that is a struct such as `{ "text": "..." }` SHALL fail schema validation before any device action. The `key` command payload remains a struct that SHALL specify exactly one named key and is not substituted by macros.

#### Scenario: Text payload is a string
- **WHEN** a `text` command payload is a non-empty string
- **THEN** the system accepts the command and delegates the text to the device layer

#### Scenario: Text payload is a struct
- **WHEN** a `text` command payload is a struct such as `{ "text": "hello" }`
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Text payload substitutes in macro bodies
- **WHEN** a repeat macro body `text` command carries `{<iter_var>}` in its payload
- **THEN** expansion replaces the token with the 0-based iteration number in each expanded command

### Requirement: AppFlow launch stores label identity and PID metadata
The system SHALL launch each `AppFlow` once when that flow begins, resolve the launched application's current PID immediately, and store app label as the logical identity with PID, bundle, and ability metadata. Multiple unique app labels MAY resolve to the same PID when the device foregrounds an already running process.

#### Scenario: PID resolves after launch
- **WHEN** an `AppFlow` launch succeeds and a PID is resolved
- **THEN** the system stores the unique app label and associated PID, bundle, and ability metadata as a tracked launched entry

#### Scenario: PID cannot be resolved after launch
- **WHEN** an `AppFlow` launch completes but no PID can be resolved
- **THEN** the system fails the benchmark before executing that flow's commands

#### Scenario: Duplicate labels are present
- **WHEN** multiple `AppFlow` entries use the same label
- **THEN** the system rejects the flow during validation before launching applications

#### Scenario: Duplicate bundle and ability are present
- **WHEN** multiple `AppFlow` entries use the same bundle and ability with different labels
- **THEN** the system allows them and associates each label with the current PID resolved after its launch

#### Scenario: AppFlow termination is requested
- **WHEN** an `AppFlow` has `terminate` equal to `True`
- **THEN** the runner requests bundle termination after that AppFlow finishes command execution

#### Scenario: AppFlow termination is not requested
- **WHEN** an `AppFlow` has `terminate` equal to `False`
- **THEN** the runner does not request bundle termination for that AppFlow

#### Scenario: Requested AppFlow termination fails
- **WHEN** an `AppFlow` has `terminate` equal to `True` and bundle termination fails
- **THEN** the system fails the benchmark

#### Scenario: AppFlow and requested termination both fail
- **WHEN** an `AppFlow` fails and requested bundle termination also fails during post-flow handling
- **THEN** the runner preserves the original AppFlow failure and SHOULD include the termination failure in a combined error when practical

### Requirement: Commands execute with action-specific semantics
The system SHALL support command actions `wait`, `snapshot`, `screenshot`, `key`, `tap`, `double_tap`, `long_tap`, `swipe`, `drag`, `fling`, `directional_fling`, `input_text`, and `text`, and schema validation MUST enforce each action's payload type before execution.

#### Scenario: Wait command executes
- **WHEN** a `wait` command has a non-negative numeric payload (integer or float)
- **THEN** the system waits for the requested number of seconds before proceeding

#### Scenario: Wait command rejects invalid payloads
- **WHEN** a `wait` command payload is negative or non-numeric
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Snapshot command executes
- **WHEN** a `snapshot` command has a valid globally unique string payload
- **THEN** the system records one snapshot timestamp, captures smaps on device for every previously launched label whose stored PID is still valid, and records pending snapshot artifacts for later transfer

#### Scenario: Screenshot command executes
- **WHEN** a `screenshot` command has a valid globally unique screenshot label payload
- **THEN** the system records one screenshot timestamp, captures the current screen to a device-local PNG file, and records a pending screenshot artifact for later transfer

#### Scenario: Key command executes
- **WHEN** a `key` command has a valid named key payload
- **THEN** the system delegates the key press to the device layer

#### Scenario: Tap command executes
- **WHEN** a `tap` command has `x_pct` and `y_pct` integer payload values in `0..100`
- **THEN** the system converts the normalized coordinate to a cached screen pixel point and delegates a tap to the device layer

#### Scenario: Double tap command executes
- **WHEN** a `double_tap` command has `x_pct` and `y_pct` integer payload values in `0..100`
- **THEN** the system converts the normalized coordinate to a cached screen pixel point and delegates a double tap to the device layer

#### Scenario: Long tap command executes
- **WHEN** a `long_tap` command has `x_pct` and `y_pct` integer payload values in `0..100`
- **THEN** the system converts the normalized coordinate to a cached screen pixel point and delegates a long tap to the device layer

#### Scenario: Swipe command executes
- **WHEN** a `swipe` command has normalized start and end coordinates and a velocity in `200..40000`
- **THEN** the system converts both normalized coordinates to cached screen pixel points and delegates a swipe to the device layer

#### Scenario: Drag command executes
- **WHEN** a `drag` command has normalized start and end coordinates and a velocity in `200..40000`
- **THEN** the system converts both normalized coordinates to cached screen pixel points and delegates a drag to the device layer

#### Scenario: Fling command executes
- **WHEN** a `fling` command has normalized start and end coordinates, a velocity in `200..40000`, and a positive step length
- **THEN** the system converts both normalized coordinates to cached screen pixel points and delegates a fling to the device layer

#### Scenario: Directional fling command executes
- **WHEN** a `directional_fling` command has a direction, a velocity in `200..40000`, and a positive step length
- **THEN** the system delegates a directional fling to the device layer

#### Scenario: Input text command executes
- **WHEN** an `input_text` command has a normalized coordinate and non-empty text
- **THEN** the system converts the normalized coordinate to a cached screen pixel point and delegates coordinate-based text input to the device layer

#### Scenario: Text command executes
- **WHEN** a `text` command has non-empty text
- **THEN** the system delegates focused text input to the device layer

#### Scenario: Unknown command is present
- **WHEN** a command action is not supported
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Velocity is outside UITest range
- **WHEN** a swipe-like command payload contains velocity less than `200` or greater than `40000`
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Step length is missing for fling
- **WHEN** a `fling` or `directional_fling` command omits `step_length`
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Step length is not positive
- **WHEN** a `fling` or `directional_fling` command contains `step_length` less than or equal to zero
- **THEN** the system fails schema validation before performing device actions

### Requirement: Benchmark execution caches screen bounds before app flows
The system SHALL read root screen bounds from the device before executing any `AppFlow` and store those bounds in the execution context for normalized coordinate conversion.

#### Scenario: Screen bounds are read successfully
- **WHEN** the benchmark begins execution after flow validation
- **THEN** the system reads device layout, parses root bounds, stores them in execution context, and only then launches the first `AppFlow`

#### Scenario: Screen bounds cannot be read
- **WHEN** device layout dumping fails before any `AppFlow` starts
- **THEN** the system fails the benchmark before launching any application

#### Scenario: Screen bounds are malformed
- **WHEN** the root layout does not contain parseable bounds
- **THEN** the system fails the benchmark before launching any application

#### Scenario: Normalized coordinate is converted
- **WHEN** a UI command requests coordinate conversion for `x_pct` and `y_pct`
- **THEN** the system converts the percent coordinate using cached root bounds and clamps the result to a positive in-bounds pixel coordinate

### Requirement: Benchmark execution prepares device environment before AppFlows
The benchmark runner SHALL prepare the device environment before launching any `AppFlow`. When reboot is enabled, it SHALL reboot the device, wait for HDC availability, wait for boot completion, wake the device, perform an upward directional fling, and send Back. Regardless of reboot setting, it SHALL disable screen timeout before AppFlows. When hihilog collection is enabled, it SHALL configure hilog before health verification and remote setup. It SHALL verify that device health is acceptable before remote output setup. It SHALL create remote output and prepare execution context before starting any enabled run-wide hilog stream. After startup validation and any enabled stream startup completes, it SHALL launch AppFlows.

#### Scenario: Reboot is enabled
- **WHEN** benchmark options have `reboot` equal to `True`
- **THEN** the runner reboots the device, waits for device availability, waits for boot completion, performs the post-reboot wake routine, configures enabled hilog, verifies device health, creates remote output, reads screen bounds, and then starts enabled hilog streaming before launching AppFlows

#### Scenario: Reboot is disabled
- **WHEN** benchmark options have `reboot` equal to `False`
- **THEN** the runner skips reboot, device availability wait, boot-complete wait, and post-reboot wake routine while still configuring enabled hilog, verifying device health, creating remote output, reading screen bounds, and starting enabled hilog streaming before launching AppFlows

#### Scenario: Post-reboot wake routine runs
- **WHEN** reboot preparation reaches boot completion
- **THEN** the runner wakes the device, performs an upward directional fling, and sends Back before disabling screen timeout

#### Scenario: Device health is acceptable
- **WHEN** benchmark execution prepares the device environment and device health reports battery capacity at least 30% and every readable nonzero thermal zone at or below 80000 millidegrees Celsius
- **THEN** the runner continues preparation and may launch AppFlows

#### Scenario: Battery capacity is too low
- **WHEN** benchmark execution prepares the device environment and device health reports battery capacity below 30%
- **THEN** the runner rejects benchmark startup before starting any enabled hilog stream, reading screen bounds, or launching AppFlows

#### Scenario: Thermal zone is too hot
- **WHEN** benchmark execution prepares the device environment and any readable nonzero thermal zone reports a temperature above 80000 millidegrees Celsius
- **THEN** the runner rejects benchmark startup before starting any enabled hilog stream, reading screen bounds, or launching AppFlows

#### Scenario: Screen timeout is disabled
- **WHEN** benchmark execution prepares the device environment
- **THEN** the runner disables screen timeout before device health verification and before launching AppFlows independent of reboot and log options

#### Scenario: Hihilog collection is enabled during preparation
- **WHEN** benchmark options have `hilog` equal to `True`
- **THEN** the runner configures hilog before remote output setup and starts the run-wide hilog stream after remote output setup, device health verification, and execution context preparation

### Requirement: Benchmark execution captures run-wide hilog
The benchmark runner SHALL collect one run-wide hilog artifact when hilog collection is enabled. It SHALL NOT clear hilog before each AppFlow and SHALL NOT dump hilog after each AppFlow. Requested AppFlow termination SHALL run after AppFlow command execution without waiting for any per-AppFlow log dump.

#### Scenario: Hihilog collection is enabled for successful benchmark
- **WHEN** all AppFlows run successfully with hilog collection enabled
- **THEN** the runner records one pending log artifact and receives it as `hilog/hilog.log` after stopping the hilog stream

#### Scenario: Hihilog collection is enabled for failing benchmark
- **WHEN** benchmark execution fails after the run-wide hilog stream has started
- **THEN** the runner attempts to stop the hilog stream before propagating the failure and still attempts to receive pending artifacts

#### Scenario: AppFlow and log stream stop both fail
- **WHEN** an AppFlow fails and stopping the hilog stream also fails
- **THEN** the runner preserves the original AppFlow failure and SHOULD include the log stream stop failure in a combined error when practical

#### Scenario: Hihilog collection is disabled
- **WHEN** benchmark options have `hilog` equal to `False`
- **THEN** the runner skips hilog configuration, stream start, stream stop, log artifact tracking, and log receive behavior

#### Scenario: Termination does not wait for per-AppFlow dump
- **WHEN** an AppFlow has `terminate` equal to `True` and hilog collection is enabled
- **THEN** the runner requests bundle termination after AppFlow command execution without performing a per-AppFlow hilog dump

### Requirement: Key command accepts named keys only
The system SHALL accept `key` commands only when the payload specifies one named key from `Home`, `Back`, or `Power`.

#### Scenario: Named key is valid
- **WHEN** a `key` command payload contains `key` equal to `Home`, `Back`, or `Power`
- **THEN** the system accepts the command and delegates the named key to the device layer

#### Scenario: Named key is invalid
- **WHEN** a `key` command payload contains any other key value
- **THEN** the system fails schema validation before performing device actions

#### Scenario: Numeric key codes are provided
- **WHEN** a `key` command payload contains numeric key codes or key combinations
- **THEN** the system fails schema validation before performing device actions

### Requirement: Snapshot skips invalid stored PIDs
The system SHALL skip a tracked launched label during snapshot collection when that label's stored `/proc/<pid>` no longer exists.

#### Scenario: Stored PID is missing during snapshot
- **WHEN** a snapshot command inspects a tracked label whose stored PID path `/proc/<pid>` is missing
- **THEN** the system skips that label for that snapshot without failing solely for that reason

#### Scenario: Stored PID exists during snapshot
- **WHEN** a snapshot command inspects a tracked label whose stored PID path `/proc/<pid>` exists
- **THEN** the system writes `/proc/<pid>/smaps` to `snapshots/<snapshot_label>/<app_label>-<snapshot_label>.smaps` as a device-local snapshot artifact and records it for later transfer

#### Scenario: All stored PIDs are missing during snapshot
- **WHEN** a snapshot command finds no tracked labels with existing stored PID paths
- **THEN** the system still records snapshot metadata for that snapshot and records no pending artifacts

#### Scenario: App relaunches outside framework control
- **WHEN** an app relaunches with a different PID outside framework control
- **THEN** the system does not rediscover the new PID and continues using only stored PIDs
