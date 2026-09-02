## ADDED Requirements

### Requirement: Smaps tag filter and renamed artifact layout are covered by tests
The test suite SHALL verify the smaps parser's tag-filter behavior, the per-file empty-filter warning, filtered-empty snapshots producing no summary row or plot point, CLI and programmatic filter wiring, and the renamed snapshot, breakdown, and plot artifact paths.

#### Scenario: Parser filter match semantics are tested
- **WHEN** tests parse smaps text with a compiled tag filter pattern
- **THEN** tests verify that only matching normalized tags appear in the breakdown and totals, that matching uses `re.match` start-anchored semantics against normalized tags (e.g. `.*\.so` matches `/system/lib64/lib.so` while `\.so$` matches nothing), and that a filter matching no tags yields `None`

#### Scenario: Filtered-empty snapshot report behavior is tested
- **WHEN** tests generate reports with a filter that matches no tags in a snapshot file
- **THEN** tests verify a per-file memmem warning containing the smaps path and the pattern, no breakdown CSV, no summary row and no plot point for that snapshot, and averaged reports treating the snapshot as absent

#### Scenario: Filtered snapshot report behavior is tested
- **WHEN** tests generate reports with a filter that matches some tags
- **THEN** tests verify the breakdown CSV, summary row, and plot points reflect only the matching tags

#### Scenario: CLI and programmatic filter options are tested
- **WHEN** tests exercise `run.py --smaps-filter` and `lib.run(smaps_filter=...)`
- **THEN** tests verify option parsing, invalid regex rejection at parse time, keyword-only propagation into report generation, and default-`None` behavior preserving unfiltered aggregation

#### Scenario: Renamed artifact layout is tested
- **WHEN** tests assert artifact paths after snapshot receive and report generation
- **THEN** tests verify `snapshots/<snapshot_label>/<app_label>-<snapshot_label>.smaps`, `breakdowns/<snapshot_label>/<app_label>-<snapshot_label>.csv`, `plots/<app_label>/<app_label>-<metric>.svg`, and the averaged breakdown and plot equivalents under `summary/`