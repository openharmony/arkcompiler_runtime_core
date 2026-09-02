## MODIFIED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that stores canonical validated flow JSON at `flow.json`, stores app metadata at `app_metadata.json`, stores snapshot metadata at `snapshots/metadata.json`, stores screenshot metadata at `screenshots/metadata.json`, stores raw smaps snapshots under `snapshots/<snapshot_label>/<app_label>.smaps`, stores screenshot artifacts under `screenshots/<screenshot_label>.png`, stores the run-wide hilog artifact at `hilog/hilog.log` when hilog collection is enabled, and stores generated memory trend plots under `plots/<app_label>/`. When repeats is greater than `1`, each iteration SHALL store its own copy of these artifacts under `iteration_{i}/`, and the run output directory SHALL additionally contain a `summary/` directory holding raw and filtered averaged reports plus discarded-iteration audit output.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a validated flow and output directory
- **THEN** the system creates the output directory, `snapshots/`, `screenshots/`, `breakdowns/`, and writes `flow.json` as canonical validated flow JSON before device actions

#### Scenario: Benchmark output preserves flow description
- **WHEN** the benchmark starts with a validated flow containing `"$desc"` metadata
- **THEN** output `flow.json` contains the same `"$desc"` metadata in the canonical validated flow JSON

#### Scenario: Benchmark output is initialized with hilog enabled
- **WHEN** the benchmark starts with hilog collection enabled
- **THEN** the system creates `hilog/` before device actions

#### Scenario: Remote output is initialized
- **WHEN** the benchmark creates remote output storage with hilog collection enabled
- **THEN** the remote output directory name under `/data/local/tmp` is derived from the local output directory path, replaces path separators and all characters outside `[A-Za-z0-9_-]` with `_`, is unique per run, and contains remote snapshot, screenshot, and hilog directories

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

#### Scenario: Run-wide hilog is received
- **WHEN** the pending run-wide device-local hilog artifact is received
- **THEN** the system stores the received hilog file at `hilog/hilog.log`

#### Scenario: Plot artifact is written
- **WHEN** report generation produces plots for an app label
- **THEN** the system stores generated SVG plot artifacts under `plots/<app_label>/`

#### Scenario: CLI input flow is canonicalized
- **WHEN** benchmark execution is started from `python run.py --flow flow.json`
- **THEN** output `flow.json` contains the canonical validated flow model and need not be byte-for-byte identical to the input file

#### Scenario: Programmatic flow is persisted
- **WHEN** benchmark execution is started through the public programmatic API
- **THEN** output `flow.json` contains the canonical validated flow model passed to execution

### Requirement: Result paths use label metadata as storage identity
The system SHALL use snapshot-label, screenshot-label, app-label, metric-name, and fixed run-wide hilog-name keyed paths so repeated bundle/ability flows can be represented even when they resolve to the same PID, generated plots can be associated with one app label and one metric, and the run-wide hilog artifact has stable identity.

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

#### Scenario: Run-wide hilog path is generated
- **WHEN** hilog collection is enabled
- **THEN** its local artifact path is `hilog/hilog.log` and its remote artifact path is `<remote_out_dir>/hilog/hilog.log`

#### Scenario: Label-derived path components are generated
- **WHEN** the system creates paths containing app labels, snapshot labels, or screenshot labels
- **THEN** it uses labels already validated as filename-safe path components

### Requirement: Pending artifacts are received before cleanup
The system SHALL track pending snapshot, screenshot, and enabled hilog artifacts in the benchmark execution context as a flat list of remote base, local base, and relative artifact path records. It SHALL receive all pending device-local artifacts into the output directory before removing the remote run directory. Artifact receive SHALL attempt every pending artifact before reporting receive failure.

#### Scenario: Pending hilog artifact is received successfully
- **WHEN** a pending run-wide hilog artifact exists on the device
- **THEN** the system receives it into `hilog/hilog.log`

#### Scenario: Pending screenshot artifact is received successfully
- **WHEN** a pending screenshot artifact exists on the device
- **THEN** the system receives it into `screenshots/<screenshot_label>.png`

#### Scenario: Pending snapshot, screenshot, and hilog artifacts are received together
- **WHEN** pending snapshot, screenshot, and run-wide hilog artifacts exist on the device
- **THEN** the system receives all pending artifacts before removing the remote run directory

#### Scenario: One artifact fails during receive
- **WHEN** one pending artifact cannot be received from the device
- **THEN** the system still attempts every other pending artifact before failing the benchmark

#### Scenario: Hilog collection is disabled
- **WHEN** hilog collection is disabled
- **THEN** the system skips pending hilog artifact tracking and receives other pending artifacts

#### Scenario: Remote cleanup fails
- **WHEN** the remote run directory cannot be removed after artifact receive
- **THEN** the system fails the benchmark

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
The system SHALL generate per-app SVG memory trend plots for Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous metrics using the same in-memory report rows used to write `summary.csv`. Plot x-axis timestamps SHALL be read from `snapshots/metadata.json` keyed by snapshot label. When `snapshots/metadata.json` is absent, the system SHALL emit a memmem warning log and skip plot generation without failing report generation.

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
- **THEN** the system emits a memmem warning log and skips plot generation without failing report generation

### Requirement: Averaged summary CSV contains per-snapshot statistics
When repeats is greater than `1`, the system SHALL generate raw and filtered aggregate reports under `out_dir/summary/`. The system SHALL generate `out_dir/summary/summary.csv` with one row per `(app_label, snapshot_label)` pair present in any iteration's per-iteration `summary.csv` using all usable iterations. The system SHALL generate `out_dir/summary/summary_filtered.csv` with the same schema after discarding default-selected outlier iterations. Each row SHALL contain `app_label`, `snapshot_label`, `n_samples`, and for each memory metric the mean, geomean, median, std, min, and max across the iterations that produced that row. Column names SHALL follow `<metric>_mean`, `<metric>_geomean`, `<metric>_median`, `<metric>_std`, `<metric>_min`, and `<metric>_max`. The system SHALL generate `out_dir/summary/outliers.csv` with one column, `discarded_iteration`, listing discarded iteration directory names. Rows present only in discarded iterations SHALL be omitted from `summary_filtered.csv`. Averaged plots and averaged breakdowns SHALL be generated under `out_dir/summary/` as described by the averaged plot and averaged breakdown requirements.

#### Scenario: Raw averaged summary row is written
- **WHEN** multiple iterations each produce a row for the same app label and snapshot label
- **THEN** raw `summary.csv` contains one row for that pair with `n_samples` equal to the number of contributing usable iterations and the six statistics computed over the corresponding per-iteration values

#### Scenario: Filtered averaged summary row is written
- **WHEN** repeated-run averaging discards one or more outlier iterations
- **THEN** `summary_filtered.csv` contains rows computed from the retained iterations only and uses the same header and statistic semantics as `summary.csv`

#### Scenario: Filtered summary is written when no iteration is discarded
- **WHEN** the filtering policy retains every usable iteration
- **THEN** `summary_filtered.csv` is still written with the same aggregate values as `summary.csv`

#### Scenario: Filtered summary omits rows present only in discarded iterations
- **WHEN** a row appears only in discarded iterations and not in any retained iteration
- **THEN** `summary_filtered.csv` omits that row

#### Scenario: Discarded iterations are audited
- **WHEN** repeated-run averaging completes
- **THEN** `outliers.csv` contains the header `discarded_iteration` and one data row for each discarded iteration directory name

#### Scenario: Whole iterations are discarded
- **WHEN** an iteration is selected as an outlier
- **THEN** every row and metric in `summary_filtered.csv` excludes that iteration while `summary.csv` still includes it

#### Scenario: Iteration outlier score uses median relative deviation
- **WHEN** repeated-run averaging scores iterations for filtering
- **THEN** each iteration score is the median of per-cell relative deviations from the median value for each comparable `(app_label, snapshot_label, metric)` cell present in that iteration

#### Scenario: Missing cells are ignored for iteration scoring
- **WHEN** an iteration lacks a row or metric cell present in another iteration
- **THEN** the missing cell does not contribute to that iteration's outlier score

#### Scenario: Zero median cell does not affect iteration score
- **WHEN** a comparable metric cell has median value `0`
- **THEN** the deviation for that cell is `0` for iteration scoring

#### Scenario: Internal filtering policy limits discarded iterations
- **WHEN** repeated-run averaging computes how many outlier iterations to discard
- **THEN** it uses the internal trim fraction, minimum retained iteration count, and minimum retained fraction to choose a deterministic discard count

#### Scenario: Iteration score ties use run order
- **WHEN** two or more candidate iterations have equal outlier scores at the discard boundary
- **THEN** the system discards tied iterations by ascending run order until the deterministic discard count is reached

#### Scenario: Geomean collapses on zero
- **WHEN** any contributing value for a metric is zero
- **THEN** the geomean for that metric is reported as zero

#### Scenario: Standard deviation and variance are undefined for a single sample
- **WHEN** an `(app_label, snapshot_label)` pair appears in exactly one retained iteration for the report being written
- **THEN** the averaged report reports `n_samples` equal to `1` and reports `std` as `0.00` while still reporting mean, geomean, median, min, and max

#### Scenario: Missing iteration summary file is skipped
- **WHEN** an iteration directory has no `summary.csv`
- **THEN** the system emits a memmem warning log and averages over the remaining iterations without failing

#### Scenario: Iteration summary files differ in snapshot rows
- **WHEN** one iteration's `summary.csv` contains a snapshot row that another iteration lacks
- **THEN** each averaged report includes the pair with `n_samples` reflecting only the retained iterations that produced that row, without failing

#### Scenario: No iteration summary files exist
- **WHEN** no iteration directory contains a `summary.csv`
- **THEN** the system fails report averaging

#### Scenario: Iteration summary row has invalid width
- **WHEN** an iteration `summary.csv` row does not contain exactly `app_label`, `snapshot_label`, and all expected metric cells
- **THEN** the system fails report averaging

#### Scenario: Averaged row order
- **WHEN** an averaged summary report is generated
- **THEN** rows are ordered by first appearance of each `(app_label, snapshot_label)` pair across retained iterations in run order

#### Scenario: Column values are typed
- **WHEN** an averaged summary report is generated
- **THEN** `n_samples`, `min`, and `max` cells are integers and `mean`, `geomean`, `median`, and `std` cells are floating-point numbers with two decimal places

### Requirement: Averaged memory trend plots are generated from the filtered summary
When repeats is greater than `1`, the system SHALL generate one averaged SVG plot per app label and memory metric under `out_dir/summary/plots/<app_label>/<metric>.svg` using the retained iterations represented by `summary/summary_filtered.csv`. Each plot SHALL plot the geomean value for that metric at each snapshot and SHALL display the corresponding std as error bars. Plot x-axis timestamps SHALL be read from `snapshots/metadata.json` of the first retained iteration in run order, keyed by snapshot label. When `snapshots/metadata.json` is absent, the system SHALL emit a memmem warning log and skip averaged plot generation without failing report averaging.

#### Scenario: Averaged plot is written per app and metric
- **WHEN** repeated-run averaging retains iterations that produced a filtered summary
- **THEN** the system writes one SVG plot under `summary/plots/<app_label>/<metric>.svg` for each memory metric with the geomean as the plotted value

#### Scenario: Averaged plot shows standard deviation error bars
- **WHEN** an averaged plot is generated for a metric with two or more contributing retained iterations
- **THEN** each plotted point displays an error bar for the std of that metric

#### Scenario: Averaged plot x-axis uses relative time
- **WHEN** plotting averaged metric rows for an app
- **THEN** the x-axis uses seconds elapsed since that app's first plotted snapshot timestamp, read from the first retained iteration's `snapshots/metadata.json`

#### Scenario: Averaged plot uses only retained iterations
- **WHEN** repeated-run averaging discards one or more outlier iterations
- **THEN** averaged plots reflect only the retained iterations and omit snapshots present only in discarded iterations

#### Scenario: Averaged plot metadata is absent
- **WHEN** the first retained iteration has no `snapshots/metadata.json`
- **THEN** the system emits a memmem warning log and skips averaged plot generation without failing report averaging

### Requirement: Breakdown CSV contains per-tag snapshot metrics
The system SHALL generate one breakdown CSV per collected snapshot under `breakdowns/<snapshot_label>/<app_label>.csv` containing tag-level Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous totals. The header SHALL be `tag,size_kb,rss_kb,pss_kb,referenced_kb,shared_kb,private_kb,swap_kb,anonymous_kb`, matching the metric column names used by `summary.csv`. Breakdown data rows SHALL be sorted by `size_kb` descending, with tag ascending as a deterministic tie-breaker.

#### Scenario: Snapshot has tag breakdowns
- **WHEN** a raw smaps snapshot is parsed
- **THEN** the system writes a breakdown CSV with header `tag,size_kb,rss_kb,pss_kb,referenced_kb,shared_kb,private_kb,swap_kb,anonymous_kb`

#### Scenario: Breakdown rows are sorted by size
- **WHEN** a breakdown CSV is written for multiple tags
- **THEN** the data rows are ordered by `size_kb` from largest to smallest

#### Scenario: Multiple snapshots exist for one app label
- **WHEN** multiple snapshots exist for the same app label
- **THEN** the system writes a separate breakdown CSV for each snapshot under that snapshot label directory

### Requirement: Averaged breakdown CSV contains per-tag statistics
When repeats is greater than `1`, the system SHALL generate one averaged breakdown CSV per `(snapshot_label, app_label)` pair present in any retained iteration under `out_dir/summary/breakdowns/<snapshot_label>/<app_label>.csv`. Each row SHALL describe one tag and SHALL contain the tag name, `n_samples`, and for each memory metric the mean, geomean, median, std, min, and max across the retained iterations that contained that tag. `n_samples` SHALL count the retained iterations whose per-iteration breakdown contained the tag. Column names SHALL follow `tag,n_samples,` followed by `<metric>_mean`, `<metric>_geomean`, `<metric>_median`, `<metric>_std`, `<metric>_min`, and `<metric>_max` for each memory metric. Data rows SHALL be sorted by `size_kb_mean` descending, with tag ascending as a deterministic tie-breaker. A retained iteration whose per-iteration breakdown file is missing SHALL be skipped without failing. Rows present only in discarded iterations SHALL be omitted. Output file creation order is not specified.

#### Scenario: Averaged breakdown row is written per tag
- **WHEN** repeated-run averaging retains iterations that produced per-iteration breakdowns
- **THEN** `summary/breakdowns/<snapshot_label>/<app_label>.csv` contains one row per tag present in any retained iteration with `n_samples` equal to the number of retained iterations containing that tag

#### Scenario: Averaged breakdown header mirrors the averaged summary
- **WHEN** an averaged breakdown CSV is generated
- **THEN** its header is `tag,n_samples,` followed by `<metric>_mean,<metric>_geomean,<metric>_median,<metric>_std,<metric>_min,<metric>_max` for each memory metric in the `summary.csv` metric order

#### Scenario: Tag missing in some retained iterations is still reported
- **WHEN** a tag appears in some but not all retained iterations
- **THEN** the averaged breakdown reports that tag with `n_samples` reflecting only the retained iterations that contained it

#### Scenario: Iteration without a breakdown file is skipped
- **WHEN** a retained iteration has no breakdown file for a `(snapshot_label, app_label)` pair present in another retained iteration
- **THEN** the system averages over the remaining retained iterations for that pair without failing

#### Scenario: Averaged breakdown omits pairs present only in discarded iterations
- **WHEN** a `(snapshot_label, app_label)` pair appears only in discarded iterations
- **THEN** `summary/breakdowns/` omits that pair

#### Scenario: Averaged breakdown rows are sorted by mean size
- **WHEN** an averaged breakdown CSV is written for multiple tags
- **THEN** the data rows are ordered by `size_kb_mean` from largest to smallest, with tag ascending for ties

#### Scenario: Averaged breakdown geomean collapses on zero
- **WHEN** any contributing value for a metric in a tag is zero
- **THEN** the geomean for that metric is reported as zero

#### Scenario: Averaged breakdown std is zero for a single sample
- **WHEN** a tag appears in exactly one retained iteration
- **THEN** the averaged breakdown reports `n_samples` equal to `1` and reports `std` as `0.00` while still reporting mean, geomean, median, min, and max
