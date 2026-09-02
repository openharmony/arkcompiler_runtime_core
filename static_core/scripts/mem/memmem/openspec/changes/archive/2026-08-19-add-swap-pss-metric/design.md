## Context

Raw `/proc/<pid>/smaps` text already arrives at `parse_smaps_text` containing `SwapPss` fields on modern kernels, but `_add_metric` only handles `Swap` and silently drops `SwapPss`. Downstream, a single constant (`SUMMARY_METRICS` in `src/report.py`) drives `summary.csv`, breakdown CSVs, averaged summaries, outlier scoring, and averaged breakdowns; a second list (`METRICS` in `src/plot.py`) drives plots. Both lists must gain the new metric, and the import graph forbids `plot.py` importing from `report.py` (report already imports plot).

## Goals / Non-Goals

**Goals:**
- Parse `SwapPss` into `swap_pss_kb` and thread it through every report artifact (summary, breakdowns, averaged reports, plots).
- Make the plot metric list derive from the shared summary metrics constant.

**Non-Goals:**
- No runner/device/schema changes (capture already records the field).
- No refactor of `SummaryRow`/`MemProfile` into auto-generated objects; explicit dataclass fields remain, and the shared list is derived from `MemProfile` fields.
- No CSV versioning or back-compat shims (breaking header change accepted).

## Decisions

- **`swap_pss_kb` field naming** matches `swap_kb`; parser maps the exact smaps field name `SwapPss` (`_METRIC_RE` already captures `SwapPss` since it matches `[A-Za-z0-9_\s]+:`).
- **`SUMMARY_METRICS` is derived from `MemProfile` in `src/smaps.py`** — `[field.name for field in dataclasses.fields(MemProfile)]` — making `MemProfile` the single source of truth: adding a metric field automatically propagates to summary CSV, breakdown CSV, and plots. `report.py` and `plot.py` both import it from `smaps.py`, an import-leaf module, so there is no cycle and `SUMMARY_METRICS` no longer needs a separate home in `types.py`.
- **`plot.py` derives `METRICS`** from `SUMMARY_METRICS` by stripping the `_kb` suffix: `"swap_pss_kb"` → `"swap_pss"` and file `swap_pss.svg`. All current metrics end in `_kb`, so this is lossless.
- **No SwapPss-absence warning** — `SwapPss` is guaranteed to be present on target devices, so the parser treats a missing field as zero without any report-generation warning.
- **Breaking CSV header change accepted** (user-approved): old output dirs fail strict header validation in `_read_summary_rows`/`_read_breakdown_rows`.

## Risks / Trade-offs

- [Strict header validation breaks old output dirs] → Accepted; internal tool with regenerated outputs.
- [`SummaryRow` fields can drift from `MemProfile` fields] → CSV headers are validated strictly against `SUMMARY_METRICS` on read; write paths build rows from `summary.total`, and mypy catches any field a new `MemProfile` metric would leave unset.
- [Fixture-driven tests hard-code expected profile values] → All `MemProfile`/`SummaryRow` constructions are updated; mypy forces completeness (dataclasses, no defaults).