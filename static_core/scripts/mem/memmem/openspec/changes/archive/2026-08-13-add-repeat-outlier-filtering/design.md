## Context

`average_reports(out_dir, stores)` currently reads each repeated iteration's `summary.csv` and writes one averaged `summary/summary.csv` using every successful iteration. This preserves all data but lets a noisy repeat distort means and inflate variance. The raw per-iteration directories remain available, so outlier filtering can be added as an additional aggregation layer without changing capture or replay behavior.

The desired filtering unit is a whole iteration. A discarded iteration should be removed from all averaged rows and metrics so the filtered report remains physically coherent: every cell in the filtered report is computed from the same retained iteration set.

## Goals / Non-Goals

**Goals:**
- Make iteration-level outlier filtering the default behavior for repeated-run aggregates.
- Keep the raw aggregate report available for auditability.
- Produce a filtered aggregate report after discarding whole iterations scored as most outlying.
- Produce a minimal audit file listing discarded iteration directory names.
- Keep filtering policy internal for v1 rather than adding CLI/API knobs.

**Non-Goals:**
- Per-cell or per-metric outlier removal.
- User-facing filter tuning flags.
- Changing per-iteration outputs, raw snapshots, raw per-iteration summaries, or flow/device execution.
- Changing the memory metrics included in summary reports.

## Decisions

### 1. Keep raw and filtered aggregates side by side

Repeated runs should produce both:

```text
summary/summary.csv
summary/summary_filtered.csv
summary/outliers.csv
```

`summary.csv` keeps the current raw aggregate over all usable iteration summaries. `summary_filtered.csv` has the same schema and statistics semantics but excludes discarded iterations. Both aggregate files are user-facing and should be documented together. `outliers.csv` has one column, `discarded_iteration`, listing iteration directory names such as `iteration_1`.

**Alternative considered:** replace `summary.csv` with filtered values. Rejected because hiding raw aggregate values makes it harder to audit the filter or compare policies later.

### 2. Score and remove whole iterations

For each iteration, compute one scalar score from all comparable `(app_label, snapshot_label, metric)` cells present in that iteration. Missing cells are ignored for that iteration's score. Discard the highest-scoring iterations. This preserves run coherence: every filtered statistic comes from the same retained set of iterations. Rows present only in discarded iterations are omitted from `summary_filtered.csv` because filtered row order and inclusion are based on retained iterations only.

**Alternative considered:** filter per cell. Rejected because it can produce a row whose metrics are averaged from different iteration subsets.

### 3. Use relative deviation from per-cell median

For each cell key `(app_label, snapshot_label, metric)`, compute the median across iterations that produced that cell. An iteration's cell deviation is:

```text
if median == 0:
  deviation = 0
else:
  deviation = abs(value - median) / median
```

All memory metrics are non-negative. Treating zero-median cells as non-discriminating avoids division by zero and means all-zero metrics such as `swap_kb` do not affect scores.

**Alternative considered:** MAD-scaled deviation. Rejected for v1 because plain relative deviation is easier to explain and tune internally.

### 4. Aggregate cell deviations by median

An iteration score is the median of its cell deviations. Median aggregation makes a whole iteration outlier only when it is broadly unusual across the run rather than because of one bad metric cell.

**Alternative considered:** mean or max aggregation. Mean is more sensitive to individual spikes; max turns any isolated cell into an iteration-level outlier.

### 5. Internal trim policy

Use internal constants:

```text
trim fraction = 0.20
minimum retained iterations = 2
minimum retained fraction = 0.60
```

The number of discarded iterations is:

```text
requested_drop = floor(n_iterations * trim_fraction)
max_drop_by_min_iterations = n_iterations - minimum_retained_iterations
max_drop_by_retained_fraction = n_iterations - ceil(n_iterations * minimum_retained_fraction)
actual_drop = min(requested_drop, max_drop_by_min_iterations, max_drop_by_retained_fraction)
```

If `actual_drop` is zero, `summary_filtered.csv` still exists and matches the raw aggregate, while `outliers.csv` contains only the header.

### 6. Deterministic ties

If iteration scores tie, discard by deterministic iteration order: sort by score descending, then by iteration run order ascending. With `iteration_{i}` directories, lower `i` wins the tie and is discarded first among equal scores.

## Risks / Trade-offs

- **Filtering can hide real app variability** → Raw `summary.csv` and every per-iteration `summary.csv` remain available.
- **Median score may ignore isolated spikes** → This is intentional for whole-iteration filtering; isolated cell anomalies can still be seen in raw data.
- **Internal constants may not fit all workloads** → Avoiding public knobs keeps v1 simple; future changes can expose policy flags after observing real data.
- **Zero-median cells do not contribute to scoring** → This prevents all-zero metrics from dominating or causing divide-by-zero; nonzero rare swap activity will not by itself trigger filtering in v1.
- **Different row presence across iterations complicates scoring** → Score only comparable cells present in each iteration; missing rows already reduce `n_samples` in aggregate reports.

## Open Questions

- Should a future diagnostic file expose iteration scores separately from the intentionally minimal `outliers.csv`?
- Should zero-median cells with nonzero values remain non-discriminating, or should future policy assign a finite deviation once real swap/nonzero-zero-median cases are observed?
