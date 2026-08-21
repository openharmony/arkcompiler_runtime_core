## Context

`src/report.py` already computes averaged summaries: `average_reports(out_dir, out_dirs)` reads each iteration's `summary.csv` into `IterationSummaryTable`, writes `summary/summary.csv`, selects discarded outlier iterations, writes `summary/summary_filtered.csv`, and audits `summary/outliers.csv`. All per-iteration reports are produced by `generate_reports(store)` which also writes per-iteration `breakdowns/` CSVs and per-app `plots/` SVGs.

The new work extends `average_reports` to additionally produce filtered-only averaged plots and averaged per-tag breakdowns, and normalizes report naming so summary and breakdown CSVs share metric column names. See proposal.md - Why for motivation; the behavior contract lives in specs/result-evidence/spec.md.

## Goals / Non-Goals

**Goals:**
- Remove `variance` from both averaged summary CSVs; report `std` as `0.00` for single-sample rows.
- Generate filtered-only averaged plots (geomean line + std error bars) under `summary/plots/<app_label>/<metric>.svg`.
- Generate filtered-only averaged per-tag breakdowns under `summary/breakdowns/<snapshot_label>/<app_label>.csv`.
- Unify breakdown metric column names with `summary.csv` (`size_kb`, `rss_kb`, ...).

**Non-Goals:**
- No change to the outlier selection algorithm itself.
- No raw (unfiltered) averaged plots or raw averaged breakdowns; only the filtered view is produced.
- No changes to per-iteration reports other than the breakdown header rename.
- No cross-app aggregate plots (still per-app per-metric only).

## Decisions

### 1. Variance removal and single-sample std
`SUMMARY_STATISTICS` loses `"variance"`; `_format_statistics` returns six values. For `n < 2`, `std` becomes `0.00` instead of `""`. A single value does not deviate from itself, so `0.00` is the correct statistic and simplifies downstream consumers (no empty-cell parsing).

- **Alternative considered:** keep empty cells for std. Rejected: produces a special empty case in CSV parsing, plots, and breakdowns for no informational gain.

### 2. Structured averaged rows shared by CSV and plots
Introduce `AveragedStatistics` (mean, geomean, median, std, min, max) and `AveragedRow` (pair, n_samples, metrics: dict[str, AveragedStatistics]). A new `_compute_averaged_rows(tables)` extracts the per-metric value-collection loop currently inside `_write_averaged_summary`. `average_reports` should compute retained averaged rows once and pass the same in-memory rows to both the filtered CSV writer and averaged plot generator, so CSV values and plotted values are computed identically.

Required implementation shape:

```text
retained tables
  -> retained averaged rows
       -> summary_filtered.csv
       -> summary/plots/<app>/<metric>.svg
```

The raw `summary.csv` can compute its own rows over all usable tables.

- **Alternative considered:** re-parse `summary_filtered.csv` for plots. Rejected: unnecessary I/O round-trip and float-formatting round-trip; in-memory rows are the single source.
- **Alternative considered:** recompute retained averaged rows independently for the filtered CSV and averaged plots. Rejected: redundant work and a weaker guarantee that CSV and plot values share one computed source.

### 3. Named app/snapshot pair type
Use one named dataclass for app/snapshot identity instead of anonymous tuples:

```python
@dataclasses.dataclass(frozen=True)
class AppSnapshotLabelPair:
    app_label: str
    snapshot_label: str
```

Reuse this type for summary rows, averaged rows, union-key logic, scoring cells, and averaged breakdown pair discovery. For breakdown paths, the path layout remains `breakdowns/<snapshot_label>/<app_label>.csv`, but the in-memory identity remains `AppSnapshotLabelPair(app_label=..., snapshot_label=...)`. This avoids the current ambiguity between `(app_label, snapshot_label)` and `(snapshot_label, app_label)` tuple ordering.

### 4. Iteration tables carry their stores
`IterationSummaryTable` should carry the source `ResultStore` in addition to its display/audit name and rows. Retained stores then come directly from retained tables instead of rebuilding a `stores_by_name` dictionary from path basenames.

Required implementation shape:

```python
@dataclasses.dataclass(frozen=True)
class IterationSummaryTable:
    name: str
    store: ResultStore
    rows: list[SummaryRow]
```

This removes basename-based lookup and avoids duplicate-name ambiguity if the averaging API is ever called with stores from different parents.

### 5. Pre-index rows by pair locally during averaging
Avoid repeated linear `_find_row` scans inside averaged row computation. `_compute_averaged_rows` should build local `dict[AppSnapshotLabelPair, SummaryRow]` indexes from each iteration table's rows, then use direct lookups while computing averaged rows. The index should remain local to the averaging helper rather than being stored on `IterationSummaryTable`, avoiding duplicate table state.

### 6. Averaged plots reuse the plot module's layout
`src/plot.py` gains `generate_averaged_plots(rows, store, timestamps)` mirroring `generate_plots`: group by app_label, filter rows whose snapshot_label is present in timestamps, sort by timestamp, x = seconds since first snapshot, and call `_write_plot` per `(metric, attribute)` from the existing `METRICS` list. `_write_plot` gains a `y_err: Sequence[float | None] | None = None` parameter; when set it uses `axes.errorbar(..., yerr=..., linestyle=":", marker="o", capsize=...)`, otherwise `axes.plot` unchanged. `_read_snapshot_timestamps` is promoted to a public `read_snapshot_timestamps(store)` so `report.py` can reuse it.

- **Alternative considered:** folding averaged data into the existing `PlotRow` protocol. Rejected: `PlotRow` is int-metric for per-iteration rows; averaged rows carry float geomeans plus std, which needs a distinct row shape and error-bar rendering.

### 7. Timestamps for averaged plots come from the first retained iteration
Averaged plots have no `snapshots/metadata.json` of their own (the `summary/` dir holds no snapshots). Timestamps are read from the first retained iteration's store, in run order, keyed by snapshot label. Because the x-axis is relative seconds since the first snapshot, the exact iteration chosen does not affect plot shape. `average_reports` uses the `ResultStore` carried by the first retained `IterationSummaryTable`.

- **Alternative considered:** snapshot order index as x-axis. Rejected: deviates from the per-iteration plots' time semantics.
- **Alternative considered:** merging timestamps across all retained iterations. Rejected: same labels across iterations; unnecessary complexity.

### 8. Unified breakdown metric naming
The per-iteration breakdown header changes from `Size_total_for_tag, Rss_total_for_tag, ...` to `["tag", *SUMMARY_METRICS]`. This lets `_read_breakdown_rows(path) -> dict[str, MemProfile]` and `_write_averaged_breakdowns` reuse `SUMMARY_METRICS`/`SUMMARY_STATISTICS` directly instead of maintaining a separate column mapping. Sort key for both per-iteration and averaged breakdowns becomes `size_kb` / `size_kb_mean`.

- **Alternative considered:** a `BREAKDOWN_METRICS` constant mapping capitalized names to fields, or header-driven parsing. Rejected: extra indirection; unifying the names removes the mapping entirely and matches the user's request to mirror `summary.csv` naming.

### 9. Averaged breakdown data flows through retained stores only
`_write_averaged_breakdowns(summary_store, retained_stores)` collects the union of `AppSnapshotLabelPair` identities across retained stores' `breakdowns/` dirs. Pairs present only in discarded iterations never appear because only retained stores are scanned. Per pair, each retained store's `local_breakdown_path(pair.snapshot_label, pair.app_label)` is read; a missing file skips that store. Per tag, metric value lists accumulate across the stores that contained the tag; `n_samples` counts those stores. Rows are formatted with the same `_format_statistics` helper and sorted by `size_kb_mean` descending, then tag lexicographically ascending for equal mean sizes. Pair discovery order is not user-visible because each pair writes a separate file.

- **Alternative considered:** re-reading `outliers.csv` to derive the retained set. Rejected: `average_reports` already holds the discarded set in memory; passing it avoids redundant I/O and parsing.

### 10. Summary-rooted ResultStore for output paths
`average_reports` creates `summary_store = ResultStore(out_dir_avg, PurePosixPath("/"))` and reuses `local_plot_path(app_label, metric)` and `local_breakdown_path(snapshot_label, app_label)` to resolve `summary/plots/...` and `summary/breakdowns/...`. `src/result.py` itself is unchanged.

## Risks / Trade-offs

- [Breaking breakdown CSV header] → This is intentional and documented as BREAKING in proposal.md; the spec's per-iteration breakdown requirement is MODIFIED accordingly, and any external consumers must regenerate.
- [Plots for single-sample rows show a point with a zero-length error bar] → Mathematically correct (std = 0.00); errorbar with yerr 0 renders a marker with no visible bar, consistent with the summary CSV value.
- [Averaged plots depend on the first retained iteration's snapshot metadata] → If metadata is absent the plots are skipped with a warning, matching the existing per-iteration behavior; snapshot labels are flow-defined and identical across iterations.
- [Multiple report outputs need identical statistics semantics] → Summary CSVs, averaged plots, and averaged breakdowns route through shared averaged-row/statistics helpers; retained summary rows are computed once and shared by filtered CSV and plots.

## Migration Plan

Behavior change ships in one commit: spec deltas updated, code changed, tests updated, README updated. Rollback is a revert of that commit; per-iteration breakdowns revert to capitalized headers and `variance` returns to averaged summaries.

## Open Questions

None.