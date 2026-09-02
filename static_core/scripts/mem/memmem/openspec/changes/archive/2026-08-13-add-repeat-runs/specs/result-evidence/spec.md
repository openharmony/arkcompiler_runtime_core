## MODIFIED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that stores canonical validated flow JSON at `flow.json`, stores app metadata at `app_metadata.json`, stores snapshot metadata at `snapshots/metadata.json`, stores screenshot metadata at `screenshots/metadata.json`, stores raw smaps snapshots under `snapshots/<snapshot_label>/<app_label>.smaps`, stores screenshot artifacts under `screenshots/<screenshot_label>.png`, stores the run-wide hilog artifact at `logs/hilog.log` when logs are enabled, and stores generated memory trend plots under `plots/<app_label>/`. When repeats is greater than `1`, each iteration SHALL store its own copy of these artifacts under `iteration_{i}/`, and the run output directory SHALL additionally contain a `summary/` directory holding the averaged report.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a validated flow and output directory
- **THEN** the system creates the output directory, `snapshots/`, `screenshots/`, `breakdowns/`, and writes `flow.json` as canonical validated flow JSON before device actions

#### Scenario: Benchmark output preserves flow description
- **WHEN** the benchmark starts with a validated flow containing `"$desc"` metadata
- **THEN** output `flow.json` contains the same `"$desc"` metadata in the canonical validated flow JSON

#### Scenario: Benchmark output is initialized with logs enabled
- **WHEN** the benchmark starts with log collection enabled
- **THEN** the system creates `logs/` before device actions

#### Scenario: Remote output is initialized
- **WHEN** the benchmark creates remote output storage with log collection enabled
- **THEN** the remote output directory name under `/data/local/tmp` is derived from the local output directory path, replaces path separators and all characters outside `[A-Za-z0-9_-]` with `_`, is unique per run, and contains remote snapshot, screenshot, and logs directories

#### Scenario: App metadata is written
- **WHEN** app-flow command execution finishes or fails after launching any apps and before artifact receive
- **THEN** the system writes `app_metadata.json` containing an `apps` array of PID, label, bundle, and ability records in app launch order

#### Scenario: Snapshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any snapshots and before artifact receive
- **THEN** the system writes `snapshots/metadata.json` containing an `artifacts` array of snapshot labels and timestamps in snapshot command order

#### Scenario: Screenshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any screenshots and before artifact receive
- **THEN** the system writes `screenshots/metadata.json` containing an `artifacts` array of screenshot labels and timestamps in screenshot command order

#### Scenario: Raw snapshot is received
- **WHEN** a pending device-local snapshot artifact is received for an app label and snapshot label
- **THEN** the system stores the received smaps file at `snapshots/<snapshot_label>/<app_label>.smaps`

#### Scenario: Raw screenshot is received
- **WHEN** a pending device-local screenshot artifact is received for a screenshot label
- **THEN** the system stores the received PNG file at `screenshots/<screenshot_label>.png`

#### Scenario: Run-wide log is received
- **WHEN** the pending run-wide device-local hilog artifact is received
- **THEN** the system stores the received log file at `logs/hilog.log`

#### Scenario: Plot artifact is written
- **WHEN** report generation produces plots for an app label
- **THEN** the system stores generated SVG plot artifacts under `plots/<app_label>/`

#### Scenario: CLI input flow is canonicalized
- **WHEN** benchmark execution is started from `python run.py --flow flow.json`
- **THEN** output `flow.json` contains the canonical validated flow model and need not be byte-for-byte identical to the input file

#### Scenario: Programmatic flow is persisted
- **WHEN** benchmark execution is started through the public programmatic API
- **THEN** output `flow.json` contains the canonical validated flow model passed to execution

### Requirement: Summary CSV contains total snapshot metrics
The system SHALL generate `summary.csv` from raw snapshots with one row per received snapshot containing app label, snapshot label, Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous totals. The CSV SHALL NOT contain PID or timestamp columns; consumers that need those values SHALL join by app label against `app_metadata.json` and by snapshot label against `snapshots/metadata.json`. Snapshot row ordering SHALL follow `snapshots/metadata.json` array order, then `app_metadata.json` array order. Internal report rows used to write the CSV SHALL NOT carry runtime PID or snapshot timestamp fields; those values remain available only in the respective metadata files.

#### Scenario: Raw snapshots exist
- **WHEN** report generation runs over an output directory with app metadata, snapshot metadata, and raw smaps snapshots
- **THEN** the system parses each existing expected snapshot and writes a corresponding total row to `summary.csv`

#### Scenario: Summary row is written
- **WHEN** report generation writes a row for a received snapshot
- **THEN** the row contains app label, snapshot label, and metric totals without PID or timestamp fields

#### Scenario: Metadata remains joinable
- **WHEN** a consumer needs PID or timestamp for a summary row
- **THEN** it can join by app label against `app_metadata.json` and by snapshot label against `snapshots/metadata.json`

#### Scenario: App metadata exists
- **WHEN** a summary row is generated for an app label
- **THEN** the system uses `app_metadata.json` to order and identify app rows without including runtime PID in internal report rows, and `summary.csv` contains no PID column

#### Scenario: Snapshot metadata exists
- **WHEN** a summary row is generated for a snapshot label
- **THEN** the system uses `snapshots/metadata.json` to order snapshot rows without including the snapshot timestamp in internal report rows, and `summary.csv` contains no timestamp column

#### Scenario: Expected raw snapshot is missing
- **WHEN** metadata references a snapshot label and app label whose expected smaps file is absent
- **THEN** the system skips that summary row without failing report generation

### Requirement: Memory trend plots are generated
The system SHALL generate per-app SVG memory trend plots for Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous metrics using the same in-memory report rows used to write `summary.csv`. Plot x-axis timestamps SHALL be read from `snapshots/metadata.json` keyed by snapshot label. When `snapshots/metadata.json` is absent, the system SHALL skip plot generation with a warning without failing report generation.

#### Scenario: Linear metric plot is generated
- **WHEN** report generation has rows for an app
- **THEN** the system writes one dotted-line SVG plot per metric under `plots/<app_label>/<metric>.svg`

#### Scenario: Log-transformed metric plot is generated
- **WHEN** report generation has rows for an app
- **THEN** the system writes one log-transformed SVG plot per metric under `plots/<app_label>/<metric>_log.svg`

#### Scenario: Zero appears in log-transformed plot
- **WHEN** a metric value is zero
- **THEN** the log-transformed plot shows that point at y = -1

#### Scenario: Negative metric value is encountered
- **WHEN** a metric value is negative
- **THEN** report generation fails because memory metrics must be non-negative

#### Scenario: Plot x-axis uses relative time
- **WHEN** plotting metric rows for an app
- **THEN** the x-axis uses seconds elapsed since that app's first plotted snapshot timestamp

#### Scenario: Plot timestamps are read from snapshot metadata
- **WHEN** plotting metric rows for an app
- **THEN** the system reads each snapshot's timestamp from `snapshots/metadata.json` keyed by snapshot label

#### Scenario: Plot points identify snapshots
- **WHEN** plotting metric rows
- **THEN** each point is associated with its snapshot label using unwrapped rotated x-axis tick labels and connected with a dotted line

#### Scenario: Dense snapshot labels avoid plotted data
- **WHEN** plotting metric rows for a flow with many snapshot labels
- **THEN** snapshot labels are rendered outside the plotted data region using rotation and layout sizing so labels do not cover the trend line or markers

#### Scenario: Plot grid lines are shown
- **WHEN** plotting metric rows
- **THEN** the system renders both x-axis and y-axis grid lines behind the plotted data

#### Scenario: Y-axis has denser tick marks
- **WHEN** plotting metric rows
- **THEN** the y-axis uses at least 10 target tick positions when possible

#### Scenario: Plotting runs without a display server
- **WHEN** reports are generated in a headless environment
- **THEN** plotting writes SVG files without requiring a GUI display

#### Scenario: No aggregate plots are generated
- **WHEN** report generation writes plot artifacts
- **THEN** it writes per-app per-metric plots only and does not write cross-app aggregate plot files

#### Scenario: Snapshot metadata is absent
- **WHEN** report generation has rows for an app but `snapshots/metadata.json` is absent
- **THEN** the system warns on stderr and skips plot generation without failing report generation

## ADDED Requirements

### Requirement: Averaged summary CSV contains per-snapshot statistics
When repeats is greater than `1`, the system SHALL generate `out_dir/summary/summary.csv` with one row per `(app_label, snapshot_label)` pair present in any iteration's per-iteration `summary.csv`. Each row SHALL contain `app_label`, `snapshot_label`, `n_samples`, and for each memory metric the mean, geomean, median, std, variance, min, and max across the iterations that produced that row. Column names SHALL follow `<metric>_mean`, `<metric>_geomean`, `<metric>_median`, `<metric>_std`, `<metric>_variance`, `<metric>_min`, and `<metric>_max`. The `summary/` directory SHALL contain only `summary.csv`. Averaged breakdowns and averaged plots are out of scope.

#### Scenario: Averaged summary row is written
- **WHEN** multiple iterations each produce a row for the same app label and snapshot label
- **THEN** the averaged `summary.csv` contains one row for that pair with `n_samples` equal to the number of contributing iterations and the seven statistics computed over the corresponding per-iteration values

#### Scenario: Geomean collapses on zero
- **WHEN** any contributing value for a metric is zero
- **THEN** the geomean for that metric is reported as zero

#### Scenario: Standard deviation and variance are undefined for a single sample
- **WHEN** an `(app_label, snapshot_label)` pair appears in exactly one iteration
- **THEN** the averaged row reports `n_samples` equal to `1` and leaves `std` and `variance` cells empty while still reporting mean, geomean, median, min, and max

#### Scenario: Missing iteration summary file is skipped
- **WHEN** an iteration directory has no `summary.csv`
- **THEN** the system reports a warning and averages over the remaining iterations without failing

#### Scenario: Iteration summary files differ in snapshot rows
- **WHEN** one iteration's `summary.csv` contains a snapshot row that another iteration lacks
- **THEN** the averaged report includes the pair with `n_samples` reflecting only the iterations that produced that row, without failing

#### Scenario: No iteration summary files exist
- **WHEN** no iteration directory contains a `summary.csv`
- **THEN** the system fails report averaging

#### Scenario: Iteration summary row has invalid width
- **WHEN** an iteration `summary.csv` row does not contain exactly `app_label`, `snapshot_label`, and all expected metric cells
- **THEN** the system fails report averaging

#### Scenario: Averaged row order
- **WHEN** the averaged `summary.csv` is generated
- **THEN** rows are ordered by first appearance of each `(app_label, snapshot_label)` pair across the iterations in run order

#### Scenario: Column values are typed
- **WHEN** the averaged `summary.csv` is generated
- **THEN** `n_samples`, `min`, and `max` cells are integers and `mean`, `geomean`, `median`, `std`, and `variance` cells are floating-point numbers with two decimal places