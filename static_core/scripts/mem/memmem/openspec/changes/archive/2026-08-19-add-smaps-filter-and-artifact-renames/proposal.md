## Why

Reports currently aggregate every smaps tag from `/proc/<pid>/smaps`, making per-artifact memory studies (e.g. "only shared libraries", "only anonymous heap") impossible without post-processing. Separately, artifacts named after a single entity (`<app>.csv`, `<metric>.svg`, `<app>.smaps`) collide in Excel and other tooling when opened across the snapshot/app directories that disambiguate them on disk — Excel rejects a second file with the same base name from a different folder.

## What Changes

- Add a `--smaps-filter` option to the `run.py` CLI: a regex string, compiled with `re.compile`, default `None` (not set). Invalid regexes are rejected by the CLI before any device action.
- Mirror it as a `smaps_filter: re.Pattern[str] | None = None` keyword-only option on `lib.run()`.
- When unset, current behavior is preserved exactly: all tags are aggregated into breakdowns, totals, summary, plots, and averaged reports.
- When set, only smaps mappings whose **normalized tag** (whitespace-stripped; `[anonymous]` for unnamed) matches the pattern via **`re.match`** (start-anchored) are included in that snapshot's breakdown CSV and in the totals that feed summary rows and plots. Documented canonical pattern for shared libraries: `.*\.so`.
- When set and a parsed snapshot has no matching tags, emit a per-file memmem warning: `smaps file {path} has no tags matching the pattern {pattern}`; write the breakdown CSV header-only; do **not** emit a summary row or plot point for that snapshot (so averaged reports treat it as absent, per existing missing-row semantics).
- **BREAKING**: rename artifacts to embed both disambiguating labels so base names are unique within the output tree:
  - snapshots (local, remote, and relative parts): `<snapshot_label>/<app_label>.smaps` → `<snapshot_label>/<app_label>-<snapshot_label>.smaps`
  - breakdowns (per-iteration and averaged): `<snapshot_label>/<app_label>.csv` → `<snapshot_label>/<app_label>-<snapshot_label>.csv`
  - plots (linear and `_log`, per-app and averaged): `<app_label>/<metric>.svg` → `<app_label>/<app_label>-<metric>.svg`
- No provenance recording of the filter; runtime warnings are the only trace. Cross-artifact and cross-iteration base-name collisions (averaged-vs-iteration breakdowns, `iteration_*` dirs) are accepted.

## Capabilities

### New Capabilities

- (none)

### Modified Capabilities

- `smaps-analysis`: the smaps parser accepts an optional compiled tag filter and restricts breakdown and total aggregation to matching mappings.
- `result-evidence`: summary rows, plot points, and breakdown CSVs derive from filtered tags; empty filtered snapshots produce a per-file warning and no summary row or plot point; snapshot, breakdown, and plot artifact file names embed both labels.
- `programmatic-interface`: `lib.run()` accepts a keyword-only `smaps_filter` runtime option mirrored from the CLI.
- `testing-support`: test-suite requirements cover filter parsing/match semantics, the empty-match warning and artifact behavior, the new paths, and the renamed artifact layout.

## Impact

- `src/smaps.py`: `parse_smaps_text(text, tag_filter=None)` — pre-compiled `re.Pattern[str] | None`; `re.match` on normalized tags; non-matching mappings excluded from `breakdown` and `total`.
- `src/report.py`: `generate_reports(store, smaps_filter=None)`; per-file warning with path and pattern; header-only breakdown CSVs; filter-emptied snapshots produce no `SummaryRow`; `_breakdown_pairs` decodes app label via `stem.removesuffix("-" + snapshot_dir.name)`.
- `src/result.py`: `local_snapshot_path`, `remote_snapshot_path`, `snapshot_relative_parts`, `local_breakdown_path`, `local_plot_path` embed both labels.
- `lib.py`: `run()` gains `smaps_filter` and passes it to `generate_reports`.
- `run.py`: `--smaps-filter` argparse option with `type=re.compile`, default `None`.
- `src/commands.py`, `src/runner.py`, `src/plot.py`: no logic changes (path helpers and relative parts centralize renames).
- No runner/device/schema changes; `average_reports` unchanged (already-filtered artifacts, missing-row semantics reused).
- Tests: `smaps_test`, `report_test`, `run_cli_test`, `lib_test`, `result_test`, `commands_test`, `runner_test`, `plot_test`.