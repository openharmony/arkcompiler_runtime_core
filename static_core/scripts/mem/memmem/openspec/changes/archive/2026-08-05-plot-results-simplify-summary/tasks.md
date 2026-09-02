## 1. Dependency and Paths

- [x] 1.1 Add matplotlib to Python dependencies.
- [x] 1.2 Add plot directory and plot path helpers to `ResultStore` for `plots/<app_label>/<metric>.svg` and `plots/<app_label>/<metric>_log.svg`.
- [x] 1.3 Add result path tests for plot directory and plot file paths.

## 2. Summary CSV Cleanup

- [x] 2.1 Remove `pid` and `timestamp` columns from `summary.csv` output while keeping them on internal report rows.
- [x] 2.2 Update report tests for the new summary CSV header and row shape.
- [x] 2.3 Ensure summary rows remain ordered by snapshot metadata order then app metadata order.

## 3. Plot Module

- [x] 3.1 Add `src/plot.py` with matplotlib configured for headless-safe SVG generation.
- [x] 3.2 Define the supported metric list: size, rss, pss, referenced, shared, private, swap, and anonymous.
- [x] 3.3 Implement app-label grouping and timestamp sorting for in-memory report rows.
- [x] 3.4 Implement per-app relative x-axis seconds from that app's first plotted snapshot.
- [x] 3.5 Implement non-negative metric validation that fails report generation on negative metric values.
- [x] 3.6 Implement linear SVG plots with dotted lines, point markers, axis labels, titles, and snapshot label annotations.
- [x] 3.7 Implement log-transformed SVG plots where zero maps to `-1` and positive values map to `log10(value)`.
- [x] 3.8 Ensure only per-app per-metric plots are generated and no aggregate plots are written.

## 4. Report Integration

- [x] 4.1 Call `generate_plots()` from `generate_reports()` using the same in-memory rows used for summary CSV generation.
- [x] 4.2 Ensure report generation creates `plots/` and per-app plot directories as needed.
- [x] 4.3 Add report tests that SVG plot files are generated from fixture result trees.
- [x] 4.4 Add plot tests for zero handling in log-transformed plots.
- [x] 4.5 Add plot/report tests for negative metric rejection.

## 5. Documentation and Specs

- [x] 5.1 Update README output layout for `plots/` and SVG plot files.
- [x] 5.2 Update README summary CSV documentation to remove `pid` and `timestamp`.
- [x] 5.3 Update delta specs if implementation behavior changes from this plan.

## 6. Verification

- [x] 6.1 Run `source ".venv/bin/activate" && make test`.
- [x] 6.2 Run `source ".venv/bin/activate" && make tests_full`.
- [x] 6.3 Run `openspec validate "plot-results-simplify-summary"`.
