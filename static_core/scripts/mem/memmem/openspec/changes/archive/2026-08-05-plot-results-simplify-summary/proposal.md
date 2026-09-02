## Why

Memory trends are hard to see from per-snapshot CSV rows alone, and `summary.csv` currently duplicates metadata already preserved in `app_metadata.json` and `snapshot_metadata.json`. Adding per-app metric plots makes growth, spikes, and leak-like patterns visible while simplifying the summary CSV into a metric table.

## What Changes

- **BREAKING**: Remove `pid` and `timestamp` columns from `summary.csv`.
- Keep `app_label` and `snapshot_label` in `summary.csv` as stable join keys into metadata files.
- Add generated memory trend plots under `plots/<app_label>/`.
- Generate SVG plots for `size`, `rss`, `pss`, `referenced`, `shared`, `private`, `swap`, and `anonymous` metrics.
- Generate both linear plots (`<metric>.svg`) and log-transformed companion plots (`<metric>_log.svg`).
- Use relative seconds since each app's first plotted snapshot as the x-axis.
- Connect plotted points with dotted lines and markers, with snapshot labels associated with points.
- Add plotting mechanics in a separate `src/plot.py` module using matplotlib with a headless-safe backend.
- Generate plots from the same in-memory report rows used to write `summary.csv`, not by reparsing the CSV.
- Fail report generation if a memory metric is negative; map zero values to `-1` in log-transformed plots.

## Capabilities

### New Capabilities

### Modified Capabilities
- `result-evidence`: summary CSV schema is simplified and generated output gains per-app SVG memory trend plots.

## Impact

- Affected code: `src/report.py`, `src/result.py`, new `src/plot.py`, tests, README, and `requirements.txt`.
- Adds matplotlib as a dependency.
- Existing consumers of `summary.csv` must stop expecting `pid` and `timestamp` columns and use `app_metadata.json` / `snapshot_metadata.json` for those values.
- Output directories gain a `plots/` tree containing derived SVG report artifacts.
