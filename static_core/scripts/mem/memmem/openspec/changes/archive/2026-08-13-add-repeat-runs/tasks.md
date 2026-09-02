## 1. Types

- [x] 1.1 Fix `src/types.py::uint` so its check and message are consistent (truly non-negative, allows zero).
- [x] 1.2 Add `positive_int` to `src/types.py` that rejects zero and negatives with an accurate message.
- [x] 1.3 Switch `run.py --repeats` to `positive_int` and update its help text to say "positive int".
- [x] 1.4 Switch `lib.run()` repeats revalidation from `uint` to `positive_int`.
- [x] 1.5 Switch `record.py --timeout` to `positive_int` (preserves the positive requirement).

## 2. Averaged report implementation

- [x] 2.1 Add shared `SUMMARY_METRICS` (size_kb, rss_kb, pss_kb, referenced_kb, shared_kb, private_kb, swap_kb, anonymous_kb) and `SUMMARY_STATISTICS` (mean, geomean, median, std, variance, min, max) constants in `src/report.py`.
- [x] 2.2 Implement a reader that parses an iteration `summary.csv` into an ordered `list[SummaryRow]` and validates the metric column header.
- [x] 2.3 Implement `average_reports(out_dir, out_dirs)` that skips missing per-iteration `summary.csv` files with a stderr warning, union-joins rows by `(app_label, snapshot_label)` in first-appearance order via row-by-row matching, and writes `out_dir/summary/summary.csv` (header `app_label, snapshot_label, n_samples` + `<metric>_<statistic>` columns).
- [x] 2.4 Implement per-metric statistics: geomean collapses to `0` on any zero value, else `statistics.geometric_mean`; `std` and `variance` are sample variants and empty for `n_samples < 2`; floats formatted to 2 decimals, integers for min/max/n_samples.
- [x] 2.5 Raise `RuntimeError` when no iteration `summary.csv` exists.
- [x] 2.6 Wire `lib.run()` to call `average_reports` only for `repeats >= 2` (already stubbed; confirm correctness).
- [x] 2.7 Sanitize remote output directory names so absolute local paths remain under `/data/local/tmp` and path parts replace non-`[A-Za-z0-9_-]` characters with `_`.
- [x] 2.8 Reject iteration `summary.csv` rows whose width does not match the expected metric columns.

## 3. Tests

- [x] 3.1 Add `test/report_test.py` coverage for `average_reports`: cell-wise stats across identical rows, geomean zero-collapse, empty std/variance for n=1, skipped missing file with warning, differing snapshot rows merged via `n_samples`, no-files failure, header naming, row order by first appearance.
- [x] 3.2 Add `test/lib_test.py` coverage for `lib.run(..., repeats=3)`: `iteration_0..2` each contain full output and `summary/summary.csv` is created.
- [x] 3.3 Add `test/run_cli_test.py` coverage: `--repeats 2` parses; `--repeats 0` and `--repeats -1` are rejected.
- [x] 3.4 Add `test/types_test.py` (or equivalent) coverage for `uint` and `positive_int` boundary behavior.
- [x] 3.5 Add a mypy fixture check (if applicable) that public `lib.run` signature accepts `repeats`.
- [x] 3.6 Add coverage for sanitized remote output directory names from absolute paths with non-compliant characters.
- [x] 3.7 Add coverage for invalid iteration `summary.csv` row width.

## 4. Docs and verification

- [x] 4.1 Update README with `--repeats` behavior: 0-indexed `iteration_{i}` dirs, `summary/summary.csv` layout, reboot-per-iteration, positive-int validation.
- [x] 4.2 Run `make tests_full` (autopep8 + tests + mypy) and ensure everything passes.