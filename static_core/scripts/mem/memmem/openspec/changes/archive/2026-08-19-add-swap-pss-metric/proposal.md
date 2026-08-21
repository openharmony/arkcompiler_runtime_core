## Why

`/proc/<pid>/smaps` on modern kernels reports `SwapPss` — the proportional share of swap usage, unlike raw `Swap` which overcounts shared pages. The capture pipeline already records the field (fixtures contain `SwapPss` lines), but the parser discards it, so summaries, breakdowns, and plots never see it. Adding it gives a meaningful "swap you're really responsible for" metric.

## What Changes

- Parse `SwapPss` from raw smaps text into a new `swap_pss_kb` field on `MemProfile` and `SummaryRow`.
- Add `swap_pss_kb` to the summary metrics list, flowing into `summary.csv`, per-tag breakdown CSVs, averaged summaries, outlier scoring, averaged breakdowns, and per-app and averaged plots (`swap_pss.svg`).
- Plot metrics derived from the shared `SUMMARY_METRICS` constant, itself derived from `MemProfile` fields (single source of truth, import-leaf module `src/smaps.py`) instead of a duplicated list.
- **BREAKING**: `summary.csv` and breakdown CSV headers gain a `swap_pss_kb` column; output directories written by older builds are not readable by this build (strict header validation) and vice versa. Accepted.

## Capabilities

### New Capabilities

- (none)

### Modified Capabilities

- `smaps-analysis`: parser produces SwapPss totals, per-tag breakdowns include it, `MemProfile` exposes `swap_pss_kb`.
- `result-evidence`: `summary.csv` rows, per-tag breakdown CSVs, and memory trend plots include SwapPss.
- `testing-support`: tests cover SwapPss parsing (and missing-as-zero) and plot/CSV presence.

## Impact

- `src/smaps.py`: `MemProfile` field, aggregation, metric parser, `SUMMARY_METRICS` derived from `MemProfile` fields.
- `src/report.py`: import shared list, `SummaryRow` field, CSV read/write.
- `src/plot.py`: `METRICS` derived from `SUMMARY_METRICS`, `PlotRow` protocol.
- Tests: `smaps_test`, `report_test`, `plot_test` (fixtures already contain `SwapPss` lines).
- No runner/device/schema changes.