## Context

`lib.run()` currently executes one flow against a device and writes a single timestamped output directory. `src/report.py` generates per-run `summary.csv`, breakdowns, and plots from raw smaps. Repeating the flow for statistical stability requires running the benchmark N times and aggregating the per-iteration summary values cell-wise. The `--repeats` plumbing (CLI arg, `lib.run` kwarg, iteration loop, remote-dir naming) is already implemented in the working tree; this change fills in the missing `average_reports` implementation and its tests. See proposal.md - Why.

## Goals / Non-Goals

**Goals:**
- Implement `average_reports(out_dir, stores)` that aggregates per-iteration `summary.csv` files into `out_dir/summary/summary.csv`.
- Cell-wise statistics: for each `(app_label, snapshot_label)` pair and each metric, compute mean, geomean, median, std, variance, min, max over the corresponding per-iteration values.
- Robustness: skip missing iteration files with a stderr warning; tolerate differing snapshot rows across iterations; fail only when no iteration `summary.csv` exists.

**Non-Goals:**
- Averaged breakdowns or averaged plots.
- Changing the per-iteration output layout or the flow schema.
- Changing reboot or fail-fast semantics (already settled in proposal.md).

## Decisions

### 1. Average from per-iteration `summary.csv`, not raw smaps
`average_reports` reads each iteration store's `summary.csv` (via `ResultStore.local_summary_path()`). Each file already holds the metric totals per snapshot; re-parsing smaps would duplicate `generate_reports` logic.
- **Alternative considered:** re-parsing smaps per iteration — rejected, adds work and risk with no benefit.

### 2. Union-join by `(app_label, snapshot_label)` pair in first-appearance order
Iterations may legitimately miss a snapshot row (dead PID). Instead of asserting identical shapes and aligning by index, keep each iteration table as a `list[SummaryRow]` preserving the original `summary.csv` row order, then emit rows for the union of pairs in first-appearance order across iterations in run order. Each row's `n_samples` is the count of iterations that produced that pair; pairs are matched by comparing `(app_label, snapshot_label)` row-by-row (O(n) per pair). The order guarantee comes from reading each `summary.csv` line-by-line into the list, not from a dict.
- **Alternative considered:** hard shape assertion + index alignment (earlier design) — rejected as too brittle; a single skipped snapshot would fail the whole average.
- **Alternative considered:** a `dict[(app_label, snapshot_label)] -> SummaryRow` per table for O(1) lookups — rejected for simplicity and to keep the reader as a plain ordered `list[SummaryRow]`; the per-pair O(n) scan is negligible at report scale.

### 3. Statistics semantics
- `geomean`: `0` if any contributing value is `0`, else `statistics.geometric_mean` (honest product rule; the stdlib function raises on zero, hence the short-circuit).
- `std` / `variance`: sample variants (`statistics.stdev`, `statistics.variance`); empty cell when `n_samples < 2` (mathematically undefined).
- `min` / `max` / `n_samples`: integers. `mean`, `geomean`, `median`, `std`, `variance`: floats formatted to 2 decimals.
- **Alternative considered:** asserting `n_samples >= 2` — rejected, reintroduces the brittleness decision 2 removes.

### 4. Wide CSV layout
Header is `app_label, snapshot_label, n_samples` followed by `<metric>_<statistic>` columns for each of the 8 metrics (size, rss, pss, referenced, shared, private, swap, anonymous) and each of the 7 statistics (mean, geomean, median, std, variance, min, max). 59 columns total. Wide format matches the existing per-iteration `summary.csv` convention and the approved design; no long-format pivot for v1.

### 5. Metrics/statistics names come from a shared constant
Extract `SUMMARY_METRICS` (the 8 metric column names) and `SUMMARY_STATISTICS` in `src/report.py` so `average_reports` and `generate_reports` stay in sync. `src/plot.py` already half-duplicates this as `METRICS`; keep that as-is to avoid scope creep, but note the coupling.

### 6. Missing-file and empty-input behavior
- An iteration store without a `summary.csv` → print a warning to stderr and skip; averaging proceeds over the remaining iterations.
- No iteration `summary.csv` at all → raise `RuntimeError` (nothing to average).
- Per-iteration metric column header that does not match the expected metrics → raise (integrity check; distinct from row-missing tolerance).

## Risks / Trade-offs

- [Row union can hide a consistently-absent snapshot] → Mitigation: `n_samples` column makes partial coverage explicit; a stderr warning accompanies any skipped file.
- [Empty `std`/`variance` cells may surprise CSV consumers] → Mitigation: documented in the spec and consistent with the approved n<2 policy.
- [Geomean zero-collapse makes one zero dominate] → Mitigation: intentional, "honest" product rule, specified in the spec; `n_samples` and the other stats give context.
- [Wide CSV (59 columns) is unwieldy] → Mitigation: accepted for v1 per the approved design; long format can be a follow-up.

## Migration Plan

- Existing `repeats == 1` runs are unaffected (no `summary/` dir, unchanged layout).
- `src/report.py` gains `average_reports` + shared constants; existing callers of `generate_reports` are untouched.
- No schema, device, or CLI breaking changes.