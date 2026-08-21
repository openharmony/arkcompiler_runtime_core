# testing-support Specification

## Purpose
Defines shared test support fakes and dependency-injection test seams used by framework tests.

## Requirements

### Requirement: Shared fake HDC supports command logging and configured results
The test suite SHALL provide a reusable fake HDC object in `test/mock/hdc.py` that records every `run`, `shell`, and `shell_raw` invocation and returns configured `HdcResult` values without executing external commands. The fake HDC SHALL support the same timeout parameter as the production HDC wrapper and MAY be configured with `timeout_after` to return `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")` when a provided timeout is too small.

#### Scenario: Default HDC result is returned
- **WHEN** a test calls `run`, `shell`, or `shell_raw` with no matching configured response
- **THEN** the fake HDC returns its configured default `HdcResult`

#### Scenario: Raw run arguments are recorded
- **WHEN** a test calls `run` with HDC arguments
- **THEN** the fake HDC records exactly those arguments without adding a `run` marker

#### Scenario: Exact HDC response is returned
- **WHEN** a test calls `run`, `shell`, or `shell_raw` with arguments matching a configured response key
- **THEN** the fake HDC returns the matching `HdcResult` and records the call

#### Scenario: Prefix HDC response is returned
- **WHEN** a test calls `run`, `shell`, or `shell_raw` with arguments matching a configured response prefix and no exact response matches
- **THEN** the fake HDC returns the matching prefix `HdcResult` and records the call

#### Scenario: Fake HDC timeout is simulated
- **WHEN** a fake HDC is configured with `timeout_after` and a command is called with a timeout smaller than that threshold
- **THEN** the fake HDC returns `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")`

### Requirement: Shared fake device models benchmark-relevant device state
The test suite SHALL provide a reusable fake device object in `test/mock/device.py` with configurable screen bounds, process state, invalid bundles, remote directories, remote files, health observations, failure flags for device operations used by benchmark execution, and fake generic child process handles for run-wide hilog streams. The fake device SHALL support reboot, boot readiness, wakeup, screen-timeout disabling, hilog configuration, hilog stream start/stop through generic child processes, app termination, and screenshot capture.

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

#### Scenario: Fake device starts hilog child process
- **WHEN** test code requests run-wide hilog streaming to a remote path on a fake device instance and hilog streaming is not configured to fail
- **THEN** that fake device instance returns a fake generic child process handle and stores fake log content in its remote files map at that path

#### Scenario: Fake hilog child process stops
- **WHEN** benchmark finalization stops a hilog child process returned by the fake device
- **THEN** the process exits without requiring multiprocessing shared memory or Unix process-group signaling

#### Scenario: Fake device hilog stream fails
- **WHEN** benchmark execution requests run-wide hilog streaming to a remote path and hilog streaming is configured to fail
- **THEN** the fake device raises a hilog streaming failure

### Requirement: Programmatic interface tests cover public facade behavior
The test suite SHALL verify the public `lib.py` facade, CLI validation behavior, CLI and programmatic macro preprocessing behavior, flow revalidation boundary, canonical flow evidence, flow description metadata behavior, recorder parsing/conversion behavior, exported public type names, and typed user-script compatibility without requiring users to import documented APIs from `src.*`.

#### Scenario: Public builders construct valid models
- **WHEN** tests construct a flow using `lib.flow()`, `lib.app_flow()`, and command wrappers
- **THEN** the resulting flow can be passed to `lib.run()` and executed with a fake device

#### Scenario: Public flow builder accepts description
- **WHEN** tests construct a flow using `lib.flow()` with `desc` set to a string
- **THEN** the resulting flow preserves the description metadata

#### Scenario: Builder validation failures are surfaced
- **WHEN** tests invoke public builder wrappers with invalid labels or invalid command payload values
- **THEN** the wrapper raises validation failure at construction time

#### Scenario: Public run revalidates mutated flow
- **WHEN** tests mutate a valid flow to break model-level invariants before passing it to `lib.run()`
- **THEN** `lib.run()` fails validation before performing device actions

#### Scenario: Public run returns no result object
- **WHEN** tests call `lib.run()` successfully
- **THEN** the call returns `None` and output is available under the caller-provided output directory

#### Scenario: Canonical flow output is tested
- **WHEN** tests run either CLI or programmatic execution
- **THEN** output `flow.json` is asserted as canonical validated flow JSON rather than a byte-for-byte input copy

#### Scenario: Canonical flow output preserves description
- **WHEN** tests run CLI or programmatic execution with a flow description
- **THEN** output `flow.json` contains the canonical `"$desc"` metadata

#### Scenario: Recorder parses and converts sample record
- **WHEN** tests pass a sample `uitest uiRecord` file containing window bounds and supported pointer/key JSON records to the recorder conversion layer
- **THEN** the generated model is a valid `Flow` containing normalized UI commands

#### Scenario: Recorder output filename format is documented by specs
- **WHEN** tests cover recorder output behavior
- **THEN** they do not need a dedicated unit test for timestamp string formatting

#### Scenario: Recorder preserves identity ordering
- **WHEN** tests convert a record containing interleaved bundle/ability identity segments, including empty identity values
- **THEN** generated app flows preserve segment order, use empty strings for empty identity fields, set `terminate: false`, and preserve input order within each segment

#### Scenario: Recorder skips unsupported pointer operations
- **WHEN** tests convert a record containing unsupported pointer operation types mixed with supported events
- **THEN** unsupported rows create no commands while supported commands remain in the generated flow

#### Scenario: Recorder rejects unknown event types
- **WHEN** tests parse a record containing an event type unsupported by the typed recorder-event schema
- **THEN** parsing fails rather than warning and continuing

#### Scenario: Old app helper is not public
- **WHEN** tests inspect the public facade exports
- **THEN** `app_flow` is exported and `app` is not available as a public helper

#### Scenario: Public type names are exported
- **WHEN** tests inspect the public facade exports
- **THEN** flow, app-flow, command, HDC, device, unprocessed flow, macro, and unprocessed command type names are exported, `preprocess_flow` and the unprocessed builder wrappers are exported, and `UnprocessedCommand`, macro payload structs such as `RepeatMacroPayload`, payload aliases such as `WaitPayload`, internal runner option types, and internal generic bases `_CommandBase` and `_MacroBase` are not exported

#### Scenario: Unprocessed builders construct valid models
- **WHEN** tests construct an unprocessed flow using `lib.unprocessed_flow`, `lib.unprocessed_app_flow`, `lib.unprocessed_snapshot`, `lib.unprocessed_screenshot`, and `lib.repeat`
- **THEN** the resulting unprocessed flow can be passed to `lib.preprocess_flow` and yields a valid canonical flow

#### Scenario: Unprocessed builder validation failures are surfaced
- **WHEN** tests invoke unprocessed builder wrappers with invalid labels, invalid `n_iter`, or invalid `iter_var`
- **THEN** the wrapper raises validation failure at construction time

#### Scenario: User script type-checks with public types
- **WHEN** tests run mypy against a fixture that imports and annotates with public `lib.py` type names
- **THEN** mypy succeeds without importing documented APIs from `src.*`

#### Scenario: Preprocessing expands macros into canonical flow
- **WHEN** tests call `lib.preprocess_flow` with an unprocessed flow containing repeat macros
- **THEN** the returned canonical `Flow` contains expanded commands with substituted labels in declared order and validates as a canonical flow

#### Scenario: Preprocessing preserves flow description
- **WHEN** tests call `lib.preprocess_flow` with an unprocessed flow containing `$desc` metadata
- **THEN** the returned canonical flow preserves the description metadata

#### Scenario: Preprocessing accepts lenient unprocessed labels
- **WHEN** tests call `lib.preprocess_flow` with an unprocessed flow whose macro body snapshot or screenshot commands use placeholder-bearing labels
- **THEN** the unprocessed flow validates and the returned canonical flow contains substituted labels that satisfy canonical label validation

#### Scenario: Preprocessing rejects top-level placeholder labels
- **WHEN** tests call `lib.preprocess_flow` with an unprocessed flow whose non-macro snapshot or screenshot command uses a placeholder label
- **THEN** unprocessed flow validation fails before expansion

#### Scenario: Preprocessing validation failures are tested
- **WHEN** tests call `lib.preprocess_flow` with invalid macro structure, nested macros, duplicate expanded labels, or string placeholders in numeric fields
- **THEN** the call fails validation before performing device actions

#### Scenario: Preprocessing wrapper revalidates input
- **WHEN** tests mutate an `UnprocessedFlow` instance into a structurally invalid state and pass it to `lib.preprocess_flow`
- **THEN** the wrapper fails validation during revalidation before any expansion

#### Scenario: Text builder produces a flat string payload
- **WHEN** tests construct a command with `lib.text()` or a flow JSON `text` command with a string payload
- **THEN** the resulting `TextCommand` carries the text string directly as its payload, and a struct `{ "text": ... }` payload fails validation

#### Scenario: Unknown keys are rejected in all schema models
- **WHEN** tests validate flows, app flows, commands, payloads, or macros carrying unknown keys, including an item carrying both `action` and `macro` keys
- **THEN** validation fails instead of silently ignoring the unknown keys

#### Scenario: Public run accepts canonical flows only
- **WHEN** tests call the public programmatic run API with a canonical flow or an unprocessed flow
- **THEN** `lib.run()` accepts canonical `Flow` models, and passing an unprocessed flow fails validation because macro-aware transformation happens only through `lib.preprocess_flow`

### Requirement: Schema regression coverage for wait payload numerics and flat text payload
The test suite SHALL verify that `wait` payload validation accepts non-negative integer and float values, rejects negative and non-numeric values, and that the flat `text` action rejects the legacy struct payload form.

#### Scenario: Wait payload accepts non-negative numeric values
- **WHEN** tests validate `wait` commands with non-negative integer and float payloads
- **THEN** the commands load and validate successfully

#### Scenario: Wait payload rejects negative and non-numeric values
- **WHEN** tests validate `wait` commands with negative or non-numeric payloads
- **THEN** schema validation fails

#### Scenario: Flat text payload rejects legacy struct form
- **WHEN** tests validate a `text` command whose payload is a struct such as `{"text": "..."}` instead of a flat string
- **THEN** schema validation fails

### Requirement: SwapPss parsing and report output are covered by tests
The test suite SHALL verify that the smaps parser aggregates `SwapPss` fields, treats absence of `SwapPss` as zero, and that plots and CSVs expose the `swap_pss_kb` metric.

#### Scenario: Parser aggregates SwapPss
- **WHEN** tests parse smaps text whose mappings contain `SwapPss` values
- **THEN** the returned profiles expose summed `swap_pss_kb` values in totals and per-tag breakdowns

#### Scenario: Parser treats missing SwapPss as zero
- **WHEN** tests parse smaps text without any `SwapPss` field
- **THEN** the returned profiles expose `swap_pss_kb` equal to zero

#### Scenario: Plots and CSVs include swap_pss
- **WHEN** tests generate summary, breakdown, and per-app and averaged plots
- **THEN** `summary.csv` and breakdown CSVs include the `swap_pss_kb` column and `swap_pss.svg` plots are written, with the plot metric list derived from the shared summary metrics constant

### Requirement: Averaged report tests cover statistics and robustness
The test suite SHALL cover `average_reports` across multiple per-iteration `summary.csv` and breakdown inputs. Tests SHALL verify per-`(app_label, snapshot_label)` cell-wise statistics over corresponding values, column naming and ordering, row ordering, zero-value geomean collapse, `std` as `0.00` for single-sample rows, skipped missing iteration summary files with a warning, differing snapshot rows across iterations, failure when no iteration `summary.csv` exists, failure when an iteration `summary.csv` row has an invalid width, failure when an iteration `summary.csv` header is invalid, default whole-iteration outlier filtering, retention limits, deterministic tie behavior, missing-row scoring behavior, rows present only in discarded iterations, no-discard output behavior, discarded-iteration audit output, averaged plot generation from retained rows, averaged breakdown generation from retained rows, invalid breakdown input handling, and named app/snapshot pair usage.

#### Scenario: Raw averaged statistics are computed over corresponding cells
- **WHEN** a test passes multiple per-iteration `summary.csv` tables with identical snapshot rows
- **THEN** raw `summary.csv` reports for each metric the mean, geomean, median, std, min, and max computed over that metric's values across all usable iterations, with `n_samples` equal to the iteration count

#### Scenario: Single-sample statistics report zero std
- **WHEN** an `(app_label, snapshot_label)` pair appears in exactly one contributing iteration for the report being written
- **THEN** the averaged row has `n_samples` equal to `1`, reports `std` as `0.00`, and does not contain variance columns

#### Scenario: Averaged plots are wired through report averaging
- **WHEN** a test runs `average_reports` with retained iterations that have snapshot metadata
- **THEN** averaged plot SVGs are written under `summary/plots/<app_label>/<app_label>-<metric>.svg` using retained averaged rows

#### Scenario: Averaged plots use retained rows only
- **WHEN** a test runs report averaging with one or more discarded iterations that contain distinct snapshot rows
- **THEN** averaged plots omit rows present only in discarded iterations

#### Scenario: Averaged breakdowns are generated from retained rows
- **WHEN** a test runs `average_reports` with retained per-iteration breakdown CSVs
- **THEN** averaged breakdown CSVs are written under `summary/breakdowns/<snapshot_label>/<app_label>-<snapshot_label>.csv` with per-tag `n_samples` and mean, geomean, median, std, min, and max columns

#### Scenario: Averaged breakdown rows sort by mean size
- **WHEN** an averaged breakdown CSV contains multiple tags
- **THEN** data rows are sorted by `size_kb_mean` descending, with tag lexicographically ascending for equal mean sizes

#### Scenario: Invalid breakdown header is rejected
- **WHEN** a retained iteration breakdown CSV has an unexpected header
- **THEN** report averaging fails with a validation error

#### Scenario: Invalid breakdown row width is rejected
- **WHEN** a retained iteration breakdown CSV row contains too few or too many cells
- **THEN** report averaging fails with a validation error

#### Scenario: Named app/snapshot pair type avoids tuple-order ambiguity
- **WHEN** tests exercise averaged summary, scoring, plot, and breakdown paths
- **THEN** those paths use a shared named app/snapshot identity type rather than anonymous `(app_label, snapshot_label)` or `(snapshot_label, app_label)` tuples

#### Scenario: Filtered averaged statistics exclude discarded iterations
- **WHEN** a test supplies repeated summaries where one or more iterations receive the highest outlier scores
- **THEN** `summary_filtered.csv` reports statistics computed from retained iterations only

#### Scenario: Discarded iterations are written to audit file
- **WHEN** a test runs repeated-report averaging with discarded iterations
- **THEN** `outliers.csv` contains one `discarded_iteration` column and one row per discarded iteration name

#### Scenario: No-discard output files are still written
- **WHEN** a test runs repeated-report averaging where the filter discards no iterations
- **THEN** `summary_filtered.csv` is written and `outliers.csv` contains only the header row

#### Scenario: Filtering preserves whole-iteration consistency
- **WHEN** an iteration is discarded by the filter
- **THEN** every metric and every snapshot row in `summary_filtered.csv` excludes that same iteration

#### Scenario: Retention limits are enforced
- **WHEN** a test supplies a repeat count where the configured trim fraction would retain fewer than the minimum retained count or fraction
- **THEN** filtering discards only as many iterations as the retention limits allow

#### Scenario: Iteration score ties use run order
- **WHEN** a test supplies tied iteration scores at the discard boundary
- **THEN** the lower run index is discarded first

#### Scenario: Missing rows are ignored for iteration scoring
- **WHEN** a test supplies an iteration missing rows present in other iterations
- **THEN** the missing cells do not contribute to that iteration's score

#### Scenario: Rows only in discarded iterations are omitted from filtered output
- **WHEN** a test supplies a row that appears only in iterations discarded by filtering
- **THEN** `summary_filtered.csv` omits that row

#### Scenario: Zero-median scoring cells do not affect filtering
- **WHEN** a test supplies metric cells with median value zero
- **THEN** those cells do not increase any iteration's outlier score

#### Scenario: Geomean collapses to zero
- **WHEN** a metric has a zero value in any contributing iteration for the report being written
- **THEN** the averaged geomean for that metric is zero

#### Scenario: Missing iteration file is skipped with a warning
- **WHEN** one iteration directory lacks a `summary.csv`
- **THEN** averaging continues over the remaining iterations and reports a warning

#### Scenario: Iterations with differing snapshot rows are merged
- **WHEN** one iteration's `summary.csv` contains a snapshot row absent from another
- **THEN** the averaged report includes that pair with `n_samples` reflecting only the contributing retained iterations

#### Scenario: No iteration summary files cause failure
- **WHEN** no iteration directory contains a `summary.csv`
- **THEN** averaging fails

#### Scenario: Column naming matches metric and statistic pairs
- **WHEN** a test inspects an averaged `summary.csv` or `summary_filtered.csv` header
- **THEN** it contains `app_label`, `snapshot_label`, `n_samples`, and `<metric>_<statistic>` columns for every memory metric and the statistics mean, geomean, median, std, min, and max

#### Scenario: Iteration summary header is checked
- **WHEN** a test passes an iteration `summary.csv` with an unexpected header
- **THEN** averaging fails with a validation error

#### Scenario: Iteration summary row width is checked
- **WHEN** a test passes an iteration `summary.csv` containing too few or too many row cells
- **THEN** averaging fails with a validation error

#### Scenario: Averaged row order follows first appearance
- **WHEN** a test supplies iterations with snapshot rows in different orders
- **THEN** averaged rows are ordered by first appearance of each pair across retained iterations in run order

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

### Requirement: Smaps tag filter and renamed artifact layout are covered by tests
The test suite SHALL verify the smaps parser's tag-filter behavior, the per-file empty-filter warning, filtered-empty snapshots producing no summary row or plot point, CLI and programmatic filter wiring, and the renamed snapshot, breakdown, and plot artifact paths.

#### Scenario: Parser filter match semantics are tested
- **WHEN** tests parse smaps text with a compiled tag filter pattern
- **THEN** tests verify that only matching normalized tags appear in the breakdown and totals, that matching uses `re.match` start-anchored semantics against normalized tags (e.g. `.*\.so` matches `/system/lib64/lib.so` while `\.so$` matches nothing), and that a filter matching no tags yields `None`

#### Scenario: Filtered-empty snapshot report behavior is tested
- **WHEN** tests generate reports with a filter that matches no tags in a snapshot file
- **THEN** tests verify a per-file memmem warning containing the smaps path and the pattern, no breakdown CSV, no summary row and no plot point for that snapshot, and averaged reports treating the snapshot as absent

#### Scenario: Filtered snapshot report behavior is tested
- **WHEN** tests generate reports with a filter that matches some tags
- **THEN** tests verify the breakdown CSV, summary row, and plot points reflect only the matching tags

#### Scenario: CLI and programmatic filter options are tested
- **WHEN** tests exercise `run.py --smaps-filter` and `lib.run(smaps_filter=...)`
- **THEN** tests verify option parsing, invalid regex rejection at parse time, keyword-only propagation into report generation, and default-`None` behavior preserving unfiltered aggregation

#### Scenario: Renamed artifact layout is tested
- **WHEN** tests assert artifact paths after snapshot receive and report generation
- **THEN** tests verify `snapshots/<snapshot_label>/<app_label>-<snapshot_label>.smaps`, `breakdowns/<snapshot_label>/<app_label>-<snapshot_label>.csv`, `plots/<app_label>/<app_label>-<metric>.svg`, and the averaged breakdown and plot equivalents under `summary/`
