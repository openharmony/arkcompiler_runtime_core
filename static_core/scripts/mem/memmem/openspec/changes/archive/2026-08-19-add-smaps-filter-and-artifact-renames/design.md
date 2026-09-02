## Context

Raw `/proc/<pid>/smaps` text contains a mapping per memory region, each tagged by path (or `[anonymous]`-style bracket names when the mapping has no path). `parse_smaps_text` (src/smaps.py) aggregates every mapping into a per-tag `breakdown` plus a `total`, and `generate_reports` (src/report.py) writes those into breakdown CSVs, `summary.csv` rows, and plots. There is currently no way to restrict analysis to a subset of tags (e.g. only shared libraries) without post-processing the CSVs.

Separately, artifact file names are scoped by their directory (e.g. `breakdowns/after_start/App.csv`), so identical base names reappear in every snapshot/app directory. Tools like Excel identify files by base name and refuse to open a second `App.csv` from a different folder, which makes multi-snapshot analysis annoying. The same pattern repeats for plots (`plots/<app>/size.svg` for every app) and snapshots (`snapshots/<snap>/App.smaps` for every snapshot).

## Goals / Non-Goals

**Goals:**
- Add an optional regex tag filter, threaded from the `run.py` CLI through `lib.run()` into report generation, restricting breakdowns, summary totals, and plot points to matching mappings; warn per filtered-empty file.
- Rename snapshot, breakdown, and plot artifact files so base names embed both identifying labels and are unique across the output tree.
- Keep `average_reports` untouched — it consumes already-filtered, already-renamed CSV artifacts.

**Non-Goals:**
- No provenance recording of the filter (runtime warnings are the only trace).
- No disambiguation of cross-artifact / cross-iteration base-name collisions (averaged-vs-iteration breakdowns, `iteration_*` dirs) — accepted.
- No schema/device/runner changes; no CSV format changes.
- No migration or back-compat shims for old output dirs (breaking, accepted; old breakdown dirs incidentally still parse).

## Decisions

- **Filter lives in the parser**, applied in `parse_smaps_text(text, tag_filter: re.Pattern[str] | None = None)` at the two mapping-flush points. Non-matching mappings are excluded from both `breakdown` and `total`, preserving the invariant `total == sum(breakdown.values())` that summary rows and tests rely on.
  - *Alternative considered*: filter after parse in report.py by recomputing totals from the filtered breakdown. Rejected: duplicates summation logic, risks divergence from the parser's invariant.
- **`re.match` semantics** against the normalized tag (start-anchored; `[anonymous]`/stripped tags). The canonical shared-library pattern is `.*\.so`; a pattern like `\.so$` never matches — documented in the CLI help and specs. `re.search` was considered and rejected: `match` is the intuitive "pattern for a tag string" contract and the user explicitly chose it.
- **CLI validation at argparse time**: `--smaps-filter` uses `type=re.compile`, so an invalid regex fails before any device action (mirrors `type=positive_int` for `--repeats`). The compiled `re.Pattern[str] | None` is then passed down: `run.py` → `lib.run(..., smaps_filter=...)` → `generate_reports(store, smaps_filter=None)` → parser.
- **Filtered-empty snapshots are absent everywhere** (decision (i)): warning + header-only breakdown CSV, but no summary row and no plot point. `average_reports` then inherits existing missing-row semantics (`_compute_averaged_rows`/`_iteration_outlier_scores` already skip rows absent from an iteration's table). This avoids a marker column in `summary.csv` (option ii) and stage coupling via file contents (option iii).
- **Warning is per filtered file**: one memmem warning per (snapshot, app) whose filtered parse yields an all-zero total, formatted `smaps file {path} has no tags matching the pattern {pattern}`. Emitted by `generate_reports` (which owns logging); the parser stays pure.
- **All renames centralize in `ResultStore`** method-level templates in src/result.py; `commands.py`, `runner.py`, and `plot.py` need no logic changes since they all go through the store helpers or `snapshot_relative_parts`:
  - snapshots (local + remote + relative parts): `<app_label>-<snapshot_label>.smaps`
  - breakdowns (per-iteration and averaged, one helper): `<app_label>-<snapshot_label>.csv`
  - plots (linear and `_log`, per-app and averaged, one helper): `<app_label>-<metric>.svg`
- **`_breakdown_pairs` decodes app label via `path.stem.removesuffix("-" + snapshot_dir.name)`**. The snapshot label (directory name) is the ground truth, so labels containing `-` stay unambiguous, and old-style `App.csv` files still decode to `App` incidentally.

## Risks / Trade-offs

- [Patterns written for `match` semantics silently match nothing (`\.so$`)] → Documented in CLI help and specs; per-file empty-filter warnings surface the miss immediately.
- [Per-file warnings can be numerous for a bad filter] → Intended: each affected file is named; users can fix the filter and rerun. Warnings appear only when a filter is configured.
- [Filtered-empty iterations contribute nothing to averaged reports while missing iterations are also skipped] → Consistent and intended: both are "absent"; iteration directories remain openable as all-zero/empty per-iteration evidence except summary rows are absent. No skew is introduced because zero rows are never written.
- [Renames break reads of old output dirs] → Accepted (precedent: `swap_pss_kb` header). Only regeneration is affected; old breakdown dirs still parse via `removesuffix`.
- [Base-name collisions remain between averaged and per-iteration breakdowns and across iterations] → Accepted (user decision (a)); scope is uniqueness within one output run.
- [Excel identifies by base name; plots/SVGs have no Excel issue but are renamed anyway] → Accepted for uniformity (user decision).

## Open Questions

- (none — decisions locked during exploration)