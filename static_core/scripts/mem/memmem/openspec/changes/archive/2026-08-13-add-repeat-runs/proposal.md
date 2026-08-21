## Why

Single benchmark runs produce noisy memory measurements. Repeating the same flow several times and averaging the per-snapshot totals gives reviewers stable, statistically meaningful numbers instead of a single observation.

## What Changes

- Add `--repeats` CLI option to `run.py` (positive int, default 1).
- Add `repeats` keyword-only argument to `lib.run()` (positive int, default 1).
- For `repeats == 1`, output layout is unchanged.
- For `repeats >= 2`:
  - the flow runs `repeats` times, each iteration writing a full run output to `out_dir/iteration_{i}` (`i` from `0` to `repeats-1`, 0-indexed);
  - `out_dir/summary/summary.csv` is additionally created with per-`(app_label, snapshot_label)` averaged statistics.
- The averaged CSV is wide: one row per snapshot across iterations with `n_samples` plus mean, geomean, median, std, variance, min, and max per memory metric.
- Averaged breakdowns and averaged plots are out of scope for this change; `summary/` contains only `summary.csv`.
- `--reboot` semantics stay as currently implemented: the device is rebooted before each iteration's flow execution.
- Iteration failure remains fail-fast: a failed iteration aborts the run and no `summary/` is produced; already-written iteration directories remain on disk.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: CLI accepts `--repeats`; repeated runs produce `iteration_{i}` output dirs; reboot applies per iteration; missing iteration `summary.csv` files are skipped with a warning.
- `programmatic-interface`: `lib.run()` accepts a `repeats` keyword argument with default `1`.
- `result-evidence`: `summary/summary.csv` averaged report layout, statistics semantics (including geomean-zero collapse and n<2 empty cells), and remote output naming for iteration directories.
- `testing-support`: tests cover `average_reports` cell-wise statistics, union-join by snapshot key, zero-value geomean, n<2 cells, and missing iteration files.
- `ui-input-recording`: recorder `--timeout` uses shared positive-int parsing.

## Impact

- Affected code: `run.py`, `lib.py`, `record.py`, `src/report.py`, `src/result.py`, `src/runner.py`, `src/types.py`.
- No change to the flow schema, command set, device interaction, or per-iteration output layout.
- No breaking change for existing flows, existing `lib.run()` callers, or `repeats == 1` output.