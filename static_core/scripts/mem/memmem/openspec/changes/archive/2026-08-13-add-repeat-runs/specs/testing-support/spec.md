## ADDED Requirements

### Requirement: Averaged report tests cover statistics and robustness
The test suite SHALL cover `average_reports` across multiple per-iteration `summary.csv` inputs. Tests SHALL verify per-`(app_label, snapshot_label)` cell-wise statistics over the corresponding per-iteration values, column naming and ordering, row ordering by first appearance, zero-value geomean collapse, empty `std` and `variance` cells for single-sample rows, skipped missing iteration files with a warning, differing snapshot rows across iterations, failure when no iteration `summary.csv` exists, and failure when an iteration `summary.csv` row has an invalid width.

#### Scenario: Averaged statistics are computed over corresponding cells
- **WHEN** a test passes multiple per-iteration `summary.csv` tables with identical snapshot rows
- **THEN** the averaged `summary.csv` reports for each metric the mean, geomean, median, std, variance, min, and max computed over that metric's values across the iterations, with `n_samples` equal to the iteration count

#### Scenario: Geomean collapses to zero
- **WHEN** a metric has a zero value in any contributing iteration
- **THEN** the averaged geomean for that metric is zero

#### Scenario: Single-sample statistics leave undefined cells empty
- **WHEN** an `(app_label, snapshot_label)` pair appears in exactly one iteration
- **THEN** the averaged row has `n_samples` equal to `1` and empty `std` and `variance` cells with populated mean, geomean, median, min, and max

#### Scenario: Missing iteration file is skipped with a warning
- **WHEN** one iteration directory lacks a `summary.csv`
- **THEN** averaging continues over the remaining iterations and reports a warning

#### Scenario: Iterations with differing snapshot rows are merged
- **WHEN** one iteration's `summary.csv` contains a snapshot row absent from another
- **THEN** the averaged report includes that pair with `n_samples` reflecting only the contributing iterations

#### Scenario: No iteration summary files cause failure
- **WHEN** no iteration directory contains a `summary.csv`
- **THEN** averaging fails

#### Scenario: Column naming matches metric and statistic pairs
- **WHEN** a test inspects the averaged `summary.csv` header
- **THEN** it contains `app_label`, `snapshot_label`, `n_samples`, and `<metric>_<statistic>` columns for every memory metric and the statistics mean, geomean, median, std, variance, min, and max

#### Scenario: Iteration summary row width is checked
- **WHEN** a test passes an iteration `summary.csv` containing too few or too many row cells
- **THEN** averaging fails with a validation error

#### Scenario: Averaged row order follows first appearance
- **WHEN** a test supplies iterations with snapshot rows in different orders
- **THEN** the averaged rows are ordered by first appearance of each pair across the iterations in run order