## MODIFIED Requirements

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
- **THEN** averaged plot SVGs are written under `summary/plots/<app_label>/<metric>.svg` using retained averaged rows

#### Scenario: Averaged plots use retained rows only
- **WHEN** a test runs report averaging with one or more discarded iterations that contain distinct snapshot rows
- **THEN** averaged plots omit rows present only in discarded iterations

#### Scenario: Averaged breakdowns are generated from retained rows
- **WHEN** a test runs `average_reports` with retained per-iteration breakdown CSVs
- **THEN** averaged breakdown CSVs are written under `summary/breakdowns/<snapshot_label>/<app_label>.csv` with per-tag `n_samples` and mean, geomean, median, std, min, and max columns

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
