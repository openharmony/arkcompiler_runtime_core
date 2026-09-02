## 1. Artifact renames

- [x] 1.1 Rename snapshot artifacts in `ResultStore`: `local_snapshot_path`, `remote_snapshot_path`, and `snapshot_relative_parts` produce `<app_label>-<snapshot_label>.smaps`
- [x] 1.2 Rename breakdown artifacts in `ResultStore.local_breakdown_path` to `<app_label>-<snapshot_label>.csv` (covers per-iteration and averaged breakdowns)
- [x] 1.3 Rename plot artifacts in `ResultStore.local_plot_path` to `<app_label>-<metric>.svg` (linear and `_log`, per-app and averaged)
- [x] 1.4 Update `_breakdown_pairs` in `src/report.py` to decode `app_label = path.stem.removesuffix(f"-{snapshot_dir.name}")`
- [x] 1.5 Update path expectations in `test/result_test.py`, `test/commands_test.py`, `test/runner_test.py` (including `fail_recv_names` entries), and `test/report_test.py` (breakdown/plot/snapshot asserts, averaged breakdown paths)

## 2. Parser filter

- [x] 2.1 Add `tag_filter: re.Pattern[str] | None = None` parameter to `parse_smaps_text` in `src/smaps.py`; exclude non-matching mappings from `breakdown` and `total` using `re.match` on the normalized tag
- [x] 2.2 Add `smaps_filter: re.Pattern[str] | None = None` parameter to `generate_reports` in `src/report.py`; pass it to the parser; emit one memmem warning per filtered file when parsing yields an all-zero total, formatted `smaps file {path} has no tags matching the pattern {pattern}`; write header-only breakdown CSV and no summary row / plot point for such snapshots
- [x] 2.3 Add `smaps_filter: re.Pattern[str] | None = None` keyword-only parameter to `lib.run` and pass it into `generate_reports`
- [x] 2.4 Add `--smaps-filter` argparse option to `run.py` with `type=re.compile`, default `None`, and help text documenting `re.match` start-anchored semantics against normalized tags (canonical shared-library pattern `.*\.so`) and invalid-regex rejection

## 3. Filter tests

- [x] 3.1 Extend `test/smaps_test.py`: filter hit/miss aggregation, `re.match` anchoring (`.*\.so` vs `\.so$`), normalized-tag matching (`[anonymous]`), no-match → empty breakdown + all-zero total, `None` unchanged
- [x] 3.2 Extend `test/report_test.py`: per-file warning with path and pattern (once per filtered file), header-only breakdown CSV, no summary row, no plot point, averaging treats the snapshot as absent
- [x] 3.3 Extend `test/run_cli_test.py` and `test/lib_test.py`: `--smaps-filter` parsing and invalid-regex rejection, default `None`, `lib.run(smaps_filter=...)` propagation end-to-end (including repeats)

## 4. Verification

- [x] 4.1 Run the full test suite and confirm all tests pass
- [x] 4.2 Run mypy and autopep8 and confirm no new issues
- [x] 4.3 Run `openspec validate` and confirm the change validates