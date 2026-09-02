## ADDED Requirements

### Requirement: SwapPss parsing and report output are covered by tests
The test suite SHALL verify that the smaps parser aggregates `SwapPss` fields, treats absence of `SwapPss` as zero, and that plots and CSVs expose the `swap_pss_kb` metric.

#### Scenario: Parser aggregates SwapPss
- **WHEN** tests parse smaps text whose mappings contain `SwapPss` values
- **THEN** the returned profiles expose summed `swap_pss_kb` values in totals and per-tag breakdowns

#### Scenario: Parser treats missing SwapPss as zero
- **WHEN** tests parse smaps text without any `SwapPss` field
- **THEN** the returned profiles expose `swap_pss_kb` equal to zero

#### Scenario: Plots and CSVs include swap_pss
- **WHEN** tests generate summary, breakdown, and per-app and averaged plots
- **THEN** `summary.csv` and breakdown CSVs include the `swap_pss_kb` column and `swap_pss.svg` plots are written, with the plot metric list derived from the shared summary metrics constant