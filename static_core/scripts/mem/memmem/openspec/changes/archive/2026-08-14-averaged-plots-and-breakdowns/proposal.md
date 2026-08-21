## Why

Repeat runs already produce filtered aggregate totals (`summary/summary_filtered.csv`) and discard outlying iterations, but the averaged view is incomplete: users cannot see the cross-iteration memory trend or the per-tag breakdown after outlier filtering. The two aggregate CSV files also carry `variance`, which adds noise when `std` already conveys the same spread information.

## What Changes

- Remove the `variance` statistic from averaged summary reports. `summary/summary.csv` and `summary/summary_filtered.csv` will no longer contain `<metric>_variance` columns.
- Report `std` as `0.00` (instead of an empty cell) when `n_samples` is `1`, since a single value does not deviate from itself. This applies to averaged summaries and averaged breakdowns.
- Generate averaged memory trend plots under `summary/plots/<app_label>/<metric>.svg`, one per metric, using geomean values with std error bars, based on the filtered (retained) iterations.
- Generate averaged per-tag breakdowns under `summary/breakdowns/<snapshot_label>/<app_label>.csv`, based on the filtered (retained) iterations, with `n_samples` per tag and the same statistics as the averaged summary.
- **BREAKING** Rename breakdown CSV metric columns from `Size_total_for_tag`, `Rss_total_for_tag`, ... to the lowercase `size_kb`, `rss_kb`, ... names used by `summary.csv`, so all report files share one metric naming scheme.

## Capabilities

### New Capabilities

None. All behavior lives in the existing `result-evidence` capability.

### Modified Capabilities

- `result-evidence`: the averaged-summary requirement changes (variance removed, std becomes `0.00` for single samples, averaged plots and averaged breakdowns become in scope) and the per-iteration breakdown requirement changes (metric column names unified with `summary.csv`).
- `testing-support`: averaged report tests expand to cover averaged plot wiring, breakdown validation failures, and the shared pair/type refactor expectations.

## Impact

- `src/report.py`: remove `variance` from `SUMMARY_STATISTICS`; make single-sample `std` `0.00`; add averaged breakdown reading/writing and wire averaged plots into `average_reports`.
- `src/plot.py`: add `generate_averaged_plots` (geomean + std error bars) and expose snapshot-timestamp reading; extend `_write_plot` with error-bar support.
- `src/result.py`: unchanged; averaged outputs reuse existing path helpers via a summary-rooted `ResultStore`.
- Tests: `test/report_test.py`, `test/plot_test.py` (variance column offsets, breakdown header rename, new averaged plot/breakdown coverage).
- Specs: `openspec/specs/result-evidence/spec.md` (main spec deltas for averaged summaries, per-iteration breakdowns, and the two new averaged requirements).
- Docs: `README.md` summary/ section updated.