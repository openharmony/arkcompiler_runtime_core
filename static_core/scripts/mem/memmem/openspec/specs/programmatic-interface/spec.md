# programmatic-interface Specification

## Purpose

Defines the public Python API for constructing and running benchmark flows programmatically.

## Requirements

### Requirement: Programmatic API runs validated flows
The system SHALL provide a public `lib.py` programmatic API that runs benchmark flows constructed in Python without requiring callers to write JSON flow files. The API SHALL accept flow content explicitly, accept a device handle, accept keyword-only runtime options, revalidate the flow before execution, run evidence collection, generate reports, return `None` on success, and report failure by raising an exception. Runtime options SHALL include `out_dir`, `reboot`, `hilog`, `repeats`, and `smaps_filter`. The `out_dir` option SHALL be resolved against the caller's current working directory at run entry and atomically reserved once before any device actions for every repeat count. An already existing output directory SHALL fail the run without changing its contents. The `smaps_filter` option SHALL accept a `re.Pattern[str] | None` value used to restrict report aggregation to matching smaps tags. The previous `logs` keyword option SHALL NOT be accepted. Memmem logger setup SHALL be available separately through public `configure_logger`, `get_logger`, and `reset_logger` helpers.

#### Scenario: Programmatic run succeeds
- **WHEN** a caller passes a valid programmatically constructed flow, a device handle, and explicit `out_dir` to `lib.run()`
- **THEN** the system executes the benchmark, generates reports, writes output under the requested directory, and returns `None`

#### Scenario: Runtime options are keyword-only
- **WHEN** a caller invokes `lib.run()`
- **THEN** the caller MUST provide `out_dir` as a keyword argument and MAY provide `reboot`, `hilog`, `repeats`, and `smaps_filter` keyword arguments

#### Scenario: Runtime option defaults match CLI
- **WHEN** a caller omits `reboot`, `hilog`, `repeats`, and `smaps_filter` from `lib.run()`
- **THEN** the system uses `reboot=False`, `hilog=True`, `repeats=1`, and `smaps_filter=None`

#### Scenario: Programmatic out dir resolves against the caller's working directory
- **WHEN** a caller passes a relative `out_dir` to `lib.run()`
- **THEN** the system resolves the directory against the caller's current working directory at run entry

#### Scenario: Programmatic out dir already exists
- **WHEN** a caller passes an `out_dir` that already exists to `lib.run()`
- **THEN** the run fails before device actions and leaves every existing directory and file byte-for-byte unchanged

#### Scenario: Repeated run root already exists
- **WHEN** a caller passes `repeats` greater than `1` and an existing `out_dir` without an `iteration_0` directory
- **THEN** the run fails before device actions and does not create iteration or summary directories

#### Scenario: Programmatic flow is revalidated
- **WHEN** a caller passes a flow object whose invariants were broken after construction
- **THEN** `lib.run()` fails validation before performing device actions

#### Scenario: Programmatic run failure raises
- **WHEN** benchmark execution or report generation fails during `lib.run()`
- **THEN** the failure is reported by raising an exception rather than returning a status object

#### Scenario: Programmatic repeats greater than one
- **WHEN** a caller passes `repeats` greater than `1` to `lib.run()`
- **THEN** the system executes the flow once per iteration, writes each iteration to an `iteration_{i}` directory under `out_dir`, and writes averaged results to `out_dir/summary/`

#### Scenario: Programmatic repeats is non-positive
- **WHEN** a caller passes a non-positive `repeats` value to `lib.run()`
- **THEN** the system rejects the value before performing device actions

#### Scenario: Programmatic hilog collection is disabled
- **WHEN** a caller passes `hilog=False` to `lib.run()`
- **THEN** the system disables run-wide device hilog collection for benchmark execution

#### Scenario: Programmatic smaps filter restricts reports
- **WHEN** a caller passes a compiled pattern as `smaps_filter` to `lib.run()`
- **THEN** report generation aggregates only matching smaps tags into breakdowns, summary rows, and plots, and treats snapshots with no matching tags as absent with a per-file warning

#### Scenario: Programmatic smaps filter default does not restrict reports
- **WHEN** a caller omits `smaps_filter` from `lib.run()`
- **THEN** the system aggregates every smaps tag exactly as before

#### Scenario: Programmatic logs keyword is removed
- **WHEN** a caller passes `logs=False` or `logs=True` to `lib.run()`
- **THEN** the system rejects the unexpected keyword argument

#### Scenario: Programmatic memmem log level is configured
- **WHEN** a caller invokes `lib.configure_logger("info")` before `lib.run()`
- **THEN** the system emits memmem info logs during benchmark execution and result processing

#### Scenario: Programmatic memmem log file is configured
- **WHEN** a caller invokes `lib.configure_logger("info", log_file)` with a non-empty log file path before `lib.run()`
- **THEN** the system writes emitted memmem logs only to that file

### Requirement: Programmatic API constructs flows with thin wrappers
The system SHALL provide public `lib.py` wrapper functions for constructing benchmark flows, app flows, and command models. These wrappers SHALL construct the existing validated Pydantic schema models immediately and SHALL NOT introduce hidden defaults unless the underlying schema model defines them. The `flow` wrapper MAY accept optional description metadata. The API SHALL also provide builder wrappers for the unprocessed flow form: `unprocessed_flow`, `unprocessed_app_flow`, `unprocessed_snapshot`, `unprocessed_screenshot`, and `repeat`, following the same construction and no-hidden-defaults rules.

#### Scenario: Flow wrapper accepts app models
- **WHEN** a caller invokes `lib.flow()` with a list of app models constructed by `lib.app_flow()`
- **THEN** the system returns a validated flow model

#### Scenario: Flow wrapper accepts description metadata
- **WHEN** a caller invokes `lib.flow()` with `desc` set to a string
- **THEN** the system returns a validated flow model containing that description metadata

#### Scenario: App-flow wrapper requires explicit fields
- **WHEN** a caller invokes `lib.app_flow()`
- **THEN** the caller MUST explicitly provide label, bundle, ability, terminate, and commands

#### Scenario: Command wrappers validate immediately
- **WHEN** a caller invokes a command wrapper with invalid labels, coordinates, velocity, step length, direction, key, or text
- **THEN** the wrapper raises validation failure during construction

#### Scenario: Text wrapper constructs a flat payload command
- **WHEN** a caller invokes `lib.text()` with a string
- **THEN** the wrapper returns a `TextCommand` whose payload is the text string directly

#### Scenario: Unprocessed flow wrapper accepts unprocessed app models
- **WHEN** a caller invokes `lib.unprocessed_flow()` with a list of unprocessed app models constructed by `lib.unprocessed_app_flow()`
- **THEN** the system returns a validated unprocessed flow model

#### Scenario: Unprocessed app-flow wrapper requires explicit fields
- **WHEN** a caller invokes `lib.unprocessed_app_flow()`
- **THEN** the caller MUST explicitly provide label, bundle, ability, terminate, and commands

#### Scenario: Unprocessed snapshot and screenshot wrappers construct lenient commands
- **WHEN** a caller invokes `lib.unprocessed_snapshot()` or `lib.unprocessed_screenshot()` with a label containing characters illegal in the canonical schema
- **THEN** the wrapper constructs the lenient unprocessed command without label validation

#### Scenario: Repeat wrapper requires explicit payload fields
- **WHEN** a caller invokes `lib.repeat()`
- **THEN** the caller MUST explicitly provide `n_iter`, `iter_var`, and `commands` (which may be empty), and the wrapper validates them immediately

#### Scenario: Unprocessed builder validation failures are surfaced
- **WHEN** a caller invokes an unprocessed builder wrapper with invalid `n_iter`, invalid `iter_var`, or invalid commands
- **THEN** the wrapper raises validation failure during construction

#### Scenario: Repetitive scenarios can be generated with Python
- **WHEN** a caller uses Python loops or helper functions to assemble repeated command sequences
- **THEN** the wrapper-produced models can be combined into a valid flow for `lib.run()`

#### Scenario: Old app wrapper is not public
- **WHEN** a caller imports documented public helpers from `lib.py`
- **THEN** `app_flow` is available and `app` is not part of the public facade

### Requirement: Programmatic API exports public type names
The system SHALL expose through `lib.py` the type names needed to write mypy-checked programmatic user scripts without importing documented APIs from `src.*`. Public type exports SHALL include flow model types, command model types used by public builders, device/HDC handle types used by public helper and run signatures, the preprocessing function `preprocess_flow`, the unprocessed flow and app-flow model types `UnprocessedFlow` and `UnprocessedAppFlow`, the macro union type `Macro`, the repeat macro type `RepeatMacro`, and the unprocessed command types `UnprocessedSnapshotCommand` and `UnprocessedScreenshotCommand`. Payload structs, macro payload structs such as `RepeatMacroPayload`, the unprocessed command union `UnprocessedCommand`, and internal runner orchestration types SHALL NOT be public exports.

#### Scenario: Typed user script annotates flow helpers
- **WHEN** a user script annotates helper functions or variables with `lib.Flow`, `lib.AppFlow`, or `lib.Command`
- **THEN** mypy can resolve those names from `lib.py` without `src.*` imports

#### Scenario: Typed user script annotates device handles
- **WHEN** a user script annotates HDC or device variables with `lib.Hdc` or `lib.Device`
- **THEN** mypy can resolve those names from `lib.py` without `src.*` imports

#### Scenario: Precise command types are available
- **WHEN** a user script needs precise builder return annotations such as `lib.TapCommand` or `lib.WaitCommand`
- **THEN** those command model type names are available from the public `lib.py` facade

#### Scenario: Typed user script annotates unprocessed types
- **WHEN** a user script annotates preprocessing inputs with `lib.UnprocessedFlow`, `lib.UnprocessedAppFlow`, `lib.Macro`, `lib.RepeatMacro`, `lib.UnprocessedSnapshotCommand`, or `lib.UnprocessedScreenshotCommand`
- **THEN** mypy can resolve those names from `lib.py` without `src.*` imports

#### Scenario: Preprocessing function is available from the facade
- **WHEN** a user script calls or annotates `lib.preprocess_flow`
- **THEN** the function is available from the public `lib.py` facade without `src.*` imports

#### Scenario: Payload structs remain private
- **WHEN** a user imports documented public helpers from `lib.py`
- **THEN** payload struct names such as `TapPayload`, `RepeatMacroPayload`, and `WaitPayload` are not part of the public facade

#### Scenario: Unprocessed command union remains private
- **WHEN** a user imports documented public helpers from `lib.py`
- **THEN** the `UnprocessedCommand` union is not part of the public facade

#### Scenario: Internal runner options remain private
- **WHEN** a user imports documented public helpers from `lib.py`
- **THEN** internal orchestration types such as `BenchmarkOptions` are not part of the public facade

### Requirement: Programmatic API preprocesses unprocessed flows
The system SHALL provide a public `lib.preprocess_flow(flow: UnprocessedFlow) -> Flow` function as the only official transformation from an unprocessed flow to a canonical flow. The function SHALL revalidate its input through `UnprocessedFlow` before expanding repeat macros in place preserving command order, substitute `{<iter_var>}` with 0-based iteration numbers in the range `[0, n_iter-1]` only in repeated commands whose payload is itself a string (flat string payloads; payload structs are not traversed), preserve flow description metadata, perform no I/O and no device access, and raise `pydantic.ValidationError` for unprocessed schema errors and for post-expansion canonical validation errors. Input acceptance is enforced statically: the signature takes `UnprocessedFlow` only, so passing a canonical flow or a raw dict is a mypy error, while structurally invalid `UnprocessedFlow` instances fail the wrapper's revalidation at runtime.

#### Scenario: Preprocessing expands macros into canonical flow
- **WHEN** a caller invokes `lib.preprocess_flow()` with an unprocessed flow containing repeat macros
- **THEN** the function returns a canonical flow containing expanded commands with substituted labels in declared order

#### Scenario: Preprocessing preserves flow description
- **WHEN** a caller invokes `lib.preprocess_flow()` with an unprocessed flow containing `$desc` metadata
- **THEN** the returned canonical flow preserves the description metadata

#### Scenario: Preprocessing input is statically typed
- **WHEN** a caller passes a raw dict or a canonical flow to `lib.preprocess_flow()` instead of an `UnprocessedFlow`
- **THEN** mypy rejects the call at type-check time, and structurally invalid `UnprocessedFlow` instances fail the wrapper's revalidation with a validation error at runtime rather than being transformed

#### Scenario: Preprocessing performs no device access
- **WHEN** a caller invokes `lib.preprocess_flow()`
- **THEN** the function performs no I/O and no device operations and produces identical output for identical input

#### Scenario: Preprocessing raises on invalid input
- **WHEN** a caller invokes `lib.preprocess_flow()` with an unprocessed flow that fails unprocessed validation or macro body validation
- **THEN** the function raises a validation error before producing a canonical flow

### Requirement: Programmatic API creates device handles through public helpers
The system SHALL provide public `lib.py` helpers for constructing HDC and device handles without requiring documented user scripts to import `src.*` modules.

#### Scenario: HDC helper accepts path-like input
- **WHEN** a caller invokes `lib.get_hdc()` with a string or `pathlib.Path` HDC executable path
- **THEN** the system returns an HDC handle usable by the public device helper

#### Scenario: Device helper wraps HDC handle
- **WHEN** a caller invokes `lib.get_device()` with an HDC handle returned by `lib.get_hdc()`
- **THEN** the system returns a device handle usable by `lib.run()`

#### Scenario: Public script avoids src imports
- **WHEN** a user follows documented programmatic API examples
- **THEN** the script imports from `lib.py` and does not need to import from `src.*`
