## MODIFIED Requirements

### Requirement: Averaged report tests cover statistics and robustness
The test suite SHALL cover `average_reports` across multiple per-iteration `summary.csv` inputs. Tests SHALL verify per-`(app_label, snapshot_label)` cell-wise statistics over the corresponding per-iteration values, column naming and ordering, row ordering by first appearance, zero-value geomean collapse, empty `std` and `variance` cells for single-sample rows, skipped missing iteration files with a warning, differing snapshot rows across iterations, failure when no iteration `summary.csv` exists, failure when an iteration `summary.csv` row has an invalid width, failure when an iteration `summary.csv` header is invalid, default whole-iteration outlier filtering, retention limits, deterministic tie behavior, missing-row scoring behavior, rows present only in discarded iterations, no-discard output behavior, and discarded-iteration audit output.

#### Scenario: Raw averaged statistics are computed over corresponding cells
- **WHEN** a test passes multiple per-iteration `summary.csv` tables with identical snapshot rows
- **THEN** raw `summary.csv` reports for each metric the mean, geomean, median, std, variance, min, and max computed over that metric's values across all usable iterations, with `n_samples` equal to the iteration count

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

#### Scenario: Single-sample statistics leave undefined cells empty
- **WHEN** an `(app_label, snapshot_label)` pair appears in exactly one contributing iteration for the report being written
- **THEN** the averaged row has `n_samples` equal to `1` and empty `std` and `variance` cells with populated mean, geomean, median, min, and max

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
- **THEN** it contains `app_label`, `snapshot_label`, `n_samples`, and `<metric>_<statistic>` columns for every memory metric and the statistics mean, geomean, median, std, variance, min, and max

#### Scenario: Iteration summary header is checked
- **WHEN** a test passes an iteration `summary.csv` with an unexpected header
- **THEN** averaging fails with a validation error

#### Scenario: Iteration summary row width is checked
- **WHEN** a test passes an iteration `summary.csv` containing too few or too many row cells
- **THEN** averaging fails with a validation error

#### Scenario: Averaged row order follows first appearance
- **WHEN** a test supplies iterations with snapshot rows in different orders
- **THEN** averaged rows are ordered by first appearance of each pair across retained iterations in run order
