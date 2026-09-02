## Why

Repeated benchmark runs can contain noisy iterations caused by background activity, app state drift, or device scheduling effects. Current averaged reports include every successful iteration, so one unusually high or low run can distort mean, variance, and other statistics even when raw iteration data remains available.

## What Changes

- Add default-on iteration-level outlier filtering for repeated runs and produce a filtered aggregate report alongside the raw aggregate report.
- Keep raw aggregate output unchanged and additionally write a filtered aggregate report; README should describe both outputs rather than positioning only one as recommended.
- Score whole iterations, not individual cells, using median aggregation over per-metric relative deviations from each `(app_label, snapshot_label, metric)` cell median.
- Use internal constants for filtering policy:
  - trim fraction: `20%`
  - minimum retained iteration count: `2`
  - minimum retained fraction: approximately `60%`
- Write `summary/outliers.csv` with one column listing discarded iteration directory names.
- Preserve per-iteration directories and per-iteration `summary.csv` files exactly as captured.
- Do not expose user-facing CLI/API tuning flags in this change; filtering policy is intentionally internal for v1.

## Capabilities

### New Capabilities

### Modified Capabilities
- `result-evidence`: repeated-run averaged output now includes raw and filtered summary reports plus discarded-iteration audit output.
- `testing-support`: tests cover default iteration-level outlier filtering, retention limits, zero-median deviation behavior, and outlier audit output.

## Impact

- Affected code: `src/report.py`, `README.md`, tests for averaged reports and repeated-run outputs.
- No change to flow schema, device interaction, per-iteration output layout, or `run.py` / `lib.py` public runtime options.
- Existing repeated-run consumers can choose raw aggregate numbers from `summary.csv` or outlier-resistant numbers from `summary_filtered.csv`; both files remain available and should be documented together.
