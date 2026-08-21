## MODIFIED Requirements

### Requirement: Averaged summary CSV contains per-snapshot statistics
When repeats is greater than `1`, the system SHALL generate raw and filtered aggregate reports under `out_dir/summary/`. The system SHALL generate `out_dir/summary/summary.csv` with one row per `(app_label, snapshot_label)` pair present in any iteration's per-iteration `summary.csv` using all usable iterations. The system SHALL generate `out_dir/summary/summary_filtered.csv` with the same schema after discarding default-selected outlier iterations. Each row SHALL contain `app_label`, `snapshot_label`, `n_samples`, and for each memory metric the mean, geomean, median, std, variance, min, and max across the iterations that produced that row. Column names SHALL follow `<metric>_mean`, `<metric>_geomean`, `<metric>_median`, `<metric>_std`, `<metric>_variance`, `<metric>_min`, and `<metric>_max`. The system SHALL generate `out_dir/summary/outliers.csv` with one column, `discarded_iteration`, listing discarded iteration directory names. Rows present only in discarded iterations SHALL be omitted from `summary_filtered.csv`. Averaged breakdowns and averaged plots are out of scope.

#### Scenario: Raw averaged summary row is written
- **WHEN** multiple iterations each produce a row for the same app label and snapshot label
- **THEN** raw `summary.csv` contains one row for that pair with `n_samples` equal to the number of contributing usable iterations and the seven statistics computed over the corresponding per-iteration values

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
- **THEN** the averaged row reports `n_samples` equal to `1` and leaves `std` and `variance` cells empty while still reporting mean, geomean, median, min, and max

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
- **THEN** `n_samples`, `min`, and `max` cells are integers and `mean`, `geomean`, `median`, `std`, and `variance` cells are floating-point numbers with two decimal places
