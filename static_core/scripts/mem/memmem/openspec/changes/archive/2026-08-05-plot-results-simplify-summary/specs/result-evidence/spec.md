## MODIFIED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that copies the input `flow.json`, stores app metadata at `app_metadata.json`, stores snapshot metadata at `snapshot_metadata.json`, stores screenshot metadata at `screenshot_metadata.json`, stores raw smaps snapshots under `snapshots/<snapshot_label>/<app_label>.smaps`, stores screenshot artifacts under `screenshots/<screenshot_label>.png`, stores AppFlow log artifacts under `logs/<app_label>.log` when logs are enabled, and stores generated memory trend plots under `plots/<app_label>/`.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a flow path and output directory
- **THEN** the system creates the output directory, `snapshots/`, `screenshots/`, `breakdowns/`, and a copy of `flow.json` before device actions

#### Scenario: Benchmark output is initialized with logs enabled
- **WHEN** the benchmark starts with log collection enabled
- **THEN** the system creates `logs/` before device actions

#### Scenario: Remote output is initialized
- **WHEN** the benchmark creates remote output storage
- **THEN** the remote output directory name under `/data/local/tmp` matches the local output directory name and contains remote snapshot and screenshot directories

#### Scenario: App metadata is written
- **WHEN** app-flow command execution finishes or fails after launching any apps and before artifact receive
- **THEN** the system writes `app_metadata.json` containing an `apps` array of PID, label, bundle, and ability records in app launch order

#### Scenario: Snapshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any snapshots and before artifact receive
- **THEN** the system writes `snapshot_metadata.json` containing a `snapshots` array of snapshot labels, timestamps, and snapshot-relative artifact names in snapshot command order

#### Scenario: Screenshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any screenshots and before artifact receive
- **THEN** the system writes `screenshot_metadata.json` containing a `screenshots` array of screenshot labels, timestamps, and screenshot-relative artifact names in screenshot command order

#### Scenario: Raw snapshot is received
- **WHEN** a pending device-local snapshot artifact is received for an app label and snapshot label
- **THEN** the system stores the received smaps file at `snapshots/<snapshot_label>/<app_label>.smaps`

#### Scenario: Raw screenshot is received
- **WHEN** a pending device-local screenshot artifact is received for a screenshot label
- **THEN** the system stores the received PNG file at `screenshots/<screenshot_label>.png`

#### Scenario: Raw log is received
- **WHEN** a pending device-local log artifact is received for an app label
- **THEN** the system stores the received log file at `logs/<app_label>.log`

#### Scenario: Plot artifact is written
- **WHEN** report generation produces plots for an app label
- **THEN** the system stores generated SVG plot artifacts under `plots/<app_label>/`

### Requirement: Result paths use label metadata as storage identity
The system SHALL use snapshot-label, screenshot-label, app-label, and metric-name keyed paths so repeated bundle/ability flows can be represented even when they resolve to the same PID and generated plots can be associated with one app label and one metric.

#### Scenario: Unique labels are stored
- **WHEN** two tracked app-flow entries have different labels
- **THEN** their snapshots are stored under distinct `snapshots/<snapshot_label>/<app_label>.smaps` paths

#### Scenario: Same PID is stored for multiple labels
- **WHEN** two tracked app-flow entries resolve to the same PID with different labels
- **THEN** their metadata records preserve the shared PID under separate app labels

#### Scenario: Screenshot label path is generated
- **WHEN** a screenshot command has label `after_login`
- **THEN** its screenshot artifact path is `screenshots/after_login.png`

#### Scenario: Plot path is generated
- **WHEN** a plot is generated for app label `gallery` and metric `rss`
- **THEN** its linear plot path is `plots/gallery/rss.svg` and its log-transformed plot path is `plots/gallery/rss_log.svg`

#### Scenario: Label-derived path components are generated
- **WHEN** the system creates paths containing app labels, snapshot labels, or screenshot labels
- **THEN** it uses labels already validated as filename-safe path components

### Requirement: Summary CSV contains total snapshot metrics
The system SHALL generate `summary.csv` from raw snapshots with one row per received snapshot containing app label, snapshot label, Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous totals. The CSV SHALL NOT contain PID or timestamp columns; consumers that need those values SHALL join by app label against `app_metadata.json` and by snapshot label against `snapshot_metadata.json`. Snapshot row ordering SHALL follow `snapshot_metadata.json` array order, then `app_metadata.json` array order.

#### Scenario: Raw snapshots exist
- **WHEN** report generation runs over an output directory with app metadata, snapshot metadata, and raw smaps snapshots
- **THEN** the system parses each existing expected snapshot and writes a corresponding total row to `summary.csv`

#### Scenario: Summary row is written
- **WHEN** report generation writes a row for a received snapshot
- **THEN** the row contains app label, snapshot label, and metric totals without PID or timestamp fields

#### Scenario: Metadata remains joinable
- **WHEN** a consumer needs PID or timestamp for a summary row
- **THEN** it can join by app label against `app_metadata.json` and by snapshot label against `snapshot_metadata.json`

#### Scenario: App metadata exists
- **WHEN** a summary row is generated for an app label
- **THEN** the system uses `app_metadata.json` to include runtime PID and display metadata in internal report rows without writing PID to `summary.csv`

#### Scenario: Snapshot metadata exists
- **WHEN** a summary row is generated for a snapshot label
- **THEN** the system uses `snapshot_metadata.json` to include the snapshot timestamp in internal report rows without writing timestamp to `summary.csv`

#### Scenario: Expected raw snapshot is missing
- **WHEN** metadata references a snapshot label and app label whose expected smaps file is absent
- **THEN** the system skips that summary row without failing report generation

## ADDED Requirements

### Requirement: Memory trend plots are generated
The system SHALL generate per-app SVG memory trend plots for Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous metrics using the same in-memory report rows used to write `summary.csv`.

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

#### Scenario: Plot points identify snapshots
- **WHEN** plotting metric rows
- **THEN** each point is associated with its snapshot label and connected with a dotted line

#### Scenario: Plotting runs without a display server
- **WHEN** reports are generated in a headless environment
- **THEN** plotting writes SVG files without requiring a GUI display

#### Scenario: No aggregate plots are generated
- **WHEN** report generation writes plot artifacts
- **THEN** it writes per-app per-metric plots only and does not write cross-app aggregate plot files
