## 1. Variance removal from averaged summaries

- [x] 1.1 Remove `"variance"` from `SUMMARY_STATISTICS` in `src/report.py`
- [x] 1.2 Update `_format_statistics` to return six statistics (mean, geomean, median, std, min, max) and report `std` as `0.00` instead of `""` when `n < 2`
- [x] 1.3 Update `test/report_test.py` `test_cell_wise_statistics_across_identical_rows` stat-index offsets (variance column removed)
- [x] 1.4 Rename/update `test_std_and_variance_empty_for_single_sample` to assert `std` is `0.00` and `variance` no longer exists

## 2. Structured averaged rows

- [x] 2.1 Add `AveragedStatistics` and `AveragedRow` dataclasses in `src/report.py`
- [x] 2.2 Extract `_compute_averaged_rows(tables) -> list[AveragedRow]` from the per-metric collection loop in `_write_averaged_summary`
- [x] 2.3 Refactor `_write_averaged_summary` to format from `AveragedRow` values (six stat columns, `std` as `0.00`)
- [x] 2.4 Add `AppSnapshotLabelPair` dataclass and reuse it for summary rows, averaged rows, scoring cells, union-key logic, and averaged breakdown pair discovery.
- [x] 2.5 Update `IterationSummaryTable` to carry its source `ResultStore` and avoid basename-based retained-store lookup.
- [x] 2.6 Pre-index iteration rows locally inside `_compute_averaged_rows` by `AppSnapshotLabelPair` to avoid repeated linear `_find_row` scans during averaging without storing duplicate state on `IterationSummaryTable`.
- [x] 2.7 Compute retained averaged rows once in `average_reports` and reuse them for `summary_filtered.csv` and averaged plots.

## 3. Averaged plots

- [x] 3.1 In `src/plot.py`, promote `_read_snapshot_timestamps` to public `read_snapshot_timestamps(store)` and update `generate_plots` to use it
- [x] 3.2 Add `y_err: Sequence[float | None] | None = None` parameter to `_write_plot`; use `axes.errorbar(..., yerr=..., linestyle=":", marker="o", capsize=...)` when set, `axes.plot` otherwise
- [x] 3.3 Add `generate_averaged_plots(rows, store, timestamps)` mirroring `generate_plots` (group by app, filter to present snapshot labels, sort by timestamp, x = seconds since first snapshot) plotting geomean with std error bars per `METRICS` entry
- [x] 3.4 Wire averaged plots into `average_reports`: build `summary_store`, read timestamps from the first retained table's `ResultStore`, and call `generate_averaged_plots` with retained averaged rows computed once
- [x] 3.5 Add `test/plot_test.py` coverage for `generate_averaged_plots` (files per app/metric, error bars present, filtered-only rows, skip when no timestamps)

## 4. Unified breakdown metric naming

- [x] 4.1 Change `write_breakdown_csv` header in `src/report.py` to `["tag", *SUMMARY_METRICS]`
- [x] 4.2 Update `test/report_test.py` `test_write_breakdown_csv` expected header to the lowercase metric names
- [x] 4.3 Update the "Breakdown CSV contains per-tag snapshot metrics" requirement text in the change spec to match (already reflected in delta spec)

## 5. Averaged breakdowns

- [x] 5.1 Add `_read_breakdown_rows(path) -> dict[str, MemProfile]` validating header `["tag", *SUMMARY_METRICS]` and row width
- [x] 5.2 Add `_write_averaged_breakdowns(summary_store, retained_stores)` collecting union of `AppSnapshotLabelPair` values across retained stores, accumulating per-tag metric values with per-tag `n_samples`, formatting via shared statistics helpers, sorting rows by `size_kb_mean` descending then tag lexicographically ascending
- [x] 5.3 Wire averaged breakdowns into `average_reports` using the retained stores list
- [x] 5.4 Extend `_write_iteration` test helper in `test/report_test.py` to optionally write per-iteration breakdown CSVs
- [x] 5.5 Add `test/report_test.py` coverage: tag union, per-tag `n_samples`, sort order, missing-file skip, discarded-only pair omission, geomean-zero collapse, single-sample `std` = `0.00`
- [x] 5.6 Add tests that invalid retained breakdown headers fail report averaging.
- [x] 5.7 Add tests that invalid retained breakdown row widths fail report averaging.
- [x] 5.8 Add an `average_reports` integration test that writes snapshot metadata and verifies averaged plots are generated from retained rows only.

## 6. Spec, docs, and verification

- [x] 6.1 Update `README.md` summary/ section: drop `variance` from the statistic list, mention `summary/plots/` and `summary/breakdowns/`, note breakdown metric columns renamed
- [x] 6.2 Run `make tests_full` and fix any failures
- [x] 6.3 Run `mypy` (strict) and fix any typing errors
- [x] 6.4 Run `openspec validate --change averaged-plots-and-breakdowns --strict` (or repo equivalent) and fix any delta issues