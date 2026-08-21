## MODIFIED Requirements

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
- **THEN** the averaged row reports `n_samples` equal to `1` and reports `std` as `0.00` while still reporting mean, geomean, median, min, and max

#### Scenario: Missing iteration summary file is skipped
- **WHEN** an iteration directory has no `summary.csv`
- **THEN** the system reports a warning and averages over the remaining iterations without failing

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

## ADDED Requirements

### Requirement: Averaged memory trend plots are generated from the filtered summary
When repeats is greater than `1`, the system SHALL generate one averaged SVG plot per app label and memory metric under `out_dir/summary/plots/<app_label>/<metric>.svg` using the retained iterations represented by `summary/summary_filtered.csv`. Each plot SHALL plot the geomean value for that metric at each snapshot and SHALL display the corresponding std as error bars. Plot x-axis timestamps SHALL be read from `snapshots/metadata.json` of the first retained iteration in run order, keyed by snapshot label. When `snapshots/metadata.json` is absent, the system SHALL skip averaged plot generation with a warning without failing report averaging.

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
- **THEN** the system warns on stderr and skips averaged plot generation without failing report averaging

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