## MODIFIED Requirements

### Requirement: Programmatic API runs validated flows
The system SHALL provide a public `lib.py` programmatic API that runs benchmark flows constructed in Python without requiring callers to write JSON flow files. The API SHALL accept flow content explicitly, accept a device handle, accept keyword-only runtime options, revalidate the flow before execution, run evidence collection, generate reports, return `None` on success, and report failure by raising an exception. Runtime options SHALL include `out_dir`, `reboot`, `hilog`, `repeats`, and `smaps_filter`. The `smaps_filter` option SHALL accept a `re.Pattern[str] | None` value used to restrict report aggregation to matching smaps tags. The previous `logs` keyword option SHALL NOT be accepted. Memmem logger setup SHALL be available separately through public `configure_logger`, `get_logger`, and `reset_logger` helpers.

#### Scenario: Programmatic run succeeds
- **WHEN** a caller passes a valid programmatically constructed flow, a device handle, and explicit `out_dir` to `lib.run()`
- **THEN** the system executes the benchmark, generates reports, writes output under the requested directory, and returns `None`

#### Scenario: Runtime options are keyword-only
- **WHEN** a caller invokes `lib.run()`
- **THEN** the caller MUST provide `out_dir` as a keyword argument and MAY provide `reboot`, `hilog`, `repeats`, and `smaps_filter` keyword arguments

#### Scenario: Runtime option defaults match CLI
- **WHEN** a caller omits `reboot`, `hilog`, `repeats`, and `smaps_filter` from `lib.run()`
- **THEN** the system uses `reboot=False`, `hilog=True`, `repeats=1`, and `smaps_filter=None`

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