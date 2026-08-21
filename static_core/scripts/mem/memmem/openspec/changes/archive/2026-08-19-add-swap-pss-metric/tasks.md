## 2. Shared metric list

- [x] 2.1 Define `SUMMARY_METRICS` in `src/smaps.py` derived from `MemProfile` fields (`dataclasses.fields`); import it in `report.py` and `plot.py`
- [x] 2.2 Add `swap_pss_kb` to `SummaryRow` and thread it through `generate_reports`, `_read_summary_rows`, and `_read_breakdown_rows`
- [x] 2.3 Derive plot `METRICS` from `SUMMARY_METRICS` and add `swap_pss_kb` to the `PlotRow` protocol

## 3. Tests

- [x] 3.1 Update `smaps_test` for `swap_pss_kb` (aggregation, missing-as-zero) across all fixtures and profiles
- [x] 3.2 Update `report_test` constructions and headers; `summary.csv`, breakdown CSVs, and averaged rows expose `swap_pss_kb`
- [x] 3.3 Update `plot_test` helpers; assert `swap_pss.svg` for linear and averaged plots and that `METRICS` derives from `SUMMARY_METRICS`

## 4. Verification

- [x] 4.1 Run the full test suite and confirm all tests pass
- [x] 4.2 Run mypy and autopep8 and confirm no new issues
- [x] 4.3 Run `openspec validate` and confirm the change validates