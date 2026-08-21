## 1. Filtering Model

- [x] 1.1 Add internal constants for trim fraction `0.20`, minimum retained iteration count `2`, and minimum retained fraction `0.60` in the report averaging layer.
- [x] 1.2 Represent iteration summary tables with their iteration directory names so discarded iterations can be audited.
- [x] 1.3 Add helper logic to compute the allowed discard count from usable iteration count and retention constants.

## 2. Iteration Scoring

- [x] 2.1 Build comparable cell values for every `(app_label, snapshot_label, metric)` across usable iteration summaries.
- [x] 2.2 Compute per-cell medians and per-iteration relative deviations, treating median-zero cells as deviation `0`.
- [x] 2.3 Compute each iteration score as the median of its comparable cell deviations.
- [x] 2.4 Select the highest-scoring iterations for discard with deterministic tie ordering by score descending, then run order ascending.
- [x] 2.5 Ignore missing row/metric cells when scoring an iteration.

## 3. Report Outputs

- [x] 3.1 Refactor averaged summary writing so the same writer can produce raw and filtered aggregate reports from a chosen set of iteration tables.
- [x] 3.2 Keep `summary/summary.csv` as the raw aggregate over all usable iteration summaries.
- [x] 3.3 Add `summary/summary_filtered.csv` using retained iteration summaries after filtering; omit rows that appear only in discarded iterations.
- [x] 3.4 Add `summary/outliers.csv` with one column `discarded_iteration` and one row per discarded iteration directory name, or header only when nothing is discarded.
- [x] 3.5 Preserve existing missing-file, invalid-header, invalid-row-width, differing-row, zero-geomean, and single-sample statistics behavior for both raw and filtered reports.

## 4. Tests and Documentation

- [x] 4.1 Add tests that default filtering discards the highest-scoring whole iteration and writes `summary_filtered.csv` plus `outliers.csv`.
- [x] 4.2 Add tests that retention limits can reduce the discard count to zero or less than the trim fraction would otherwise request.
- [x] 4.3 Add tests for median-zero scoring cells contributing zero deviation.
- [x] 4.4 Add tests that discarded iterations are excluded consistently from every metric in the filtered report.
- [x] 4.5 Add tests for deterministic tie behavior by run order.
- [x] 4.6 Add tests for missing rows being ignored during iteration scoring.
- [x] 4.7 Add tests that rows present only in discarded iterations are omitted from `summary_filtered.csv`.
- [x] 4.8 Add tests that no-discard cases still write `summary_filtered.csv` and header-only `outliers.csv`.
- [x] 4.9 Preserve tests for invalid headers and invalid row widths.
- [x] 4.10 Update README repeated-run output docs to describe raw aggregate, filtered aggregate, outlier audit file, and the default internal policy without recommending only one summary file.

## 5. Verification

- [x] 5.1 Run `make tests_full`.
- [x] 5.2 Run `openspec validate "add-repeat-outlier-filtering" --type change`.
- [x] 5.3 Run `openspec validate --all`.
