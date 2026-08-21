## ADDED Requirements

## MODIFIED Requirements

### Requirement: Summary CSV contains total snapshot metrics
The system SHALL generate `summary.csv` from raw snapshots with one row per received snapshot containing app label, snapshot label, Size, Rss, Pss, Referenced, Shared, Private, Swap, SwapPss, and Anonymous totals. The CSV SHALL NOT contain PID or timestamp columns; consumers that need those values SHALL join by app label against `app_metadata.json` and by snapshot label against `snapshots/metadata.json`. Snapshot row ordering SHALL follow `snapshots/metadata.json` array order, then `app_metadata.json` array order. Internal report rows used to write the CSV SHALL NOT carry runtime PID or snapshot timestamp fields; those values remain available only in the respective metadata files.

#### Scenario: Raw snapshots exist
- **WHEN** report generation runs over an output directory with app metadata, snapshot metadata, and raw smaps snapshots
- **THEN** the system parses each existing expected snapshot and writes a corresponding total row to `summary.csv`

#### Scenario: Summary row is written
- **WHEN** report generation writes a row for a received snapshot
- **THEN** the row contains app label, snapshot label, and metric totals without PID or timestamp fields

#### Scenario: Metadata remains joinable
- **WHEN** a consumer needs PID or timestamp for a summary row
- **THEN** it can join by app label against `app_metadata.json` and by snapshot label against `snapshots/metadata.json`

#### Scenario: App metadata exists
- **WHEN** a summary row is generated for an app label
- **THEN** the system uses `app_metadata.json` to order and identify app rows without including runtime PID in internal report rows, and `summary.csv` contains no PID column

#### Scenario: Snapshot metadata exists
- **WHEN** a summary row is generated for a snapshot label
- **THEN** the system uses `snapshots/metadata.json` to order snapshot rows without including the snapshot timestamp in internal report rows, and `summary.csv` contains no timestamp column

#### Scenario: Expected raw snapshot is missing
- **WHEN** metadata references a snapshot label and app label whose expected smaps file is absent
- **THEN** the system skips that summary row without failing report generation

### Requirement: Memory trend plots are generated
The system SHALL generate per-app SVG memory trend plots for Size, Rss, Pss, Referenced, Shared, Private, Swap, SwapPss, and Anonymous metrics using the same in-memory report rows used to write `summary.csv`. Plot x-axis timestamps SHALL be read from `snapshots/metadata.json` keyed by snapshot label. When `snapshots/metadata.json` is absent, the system SHALL emit a memmem warning log and skip plot generation without failing report generation.

#### Scenario: Linear metric plot is generated
- **WHEN** report generation has rows for an app
- **THEN** the system writes one dotted-line SVG plot per metric under `plots/<app_label>/<metric>.svg`

#### Scenario: Log-transformed metric plot is generated
- **WHEN** report generation has rows for an app
- **THEN** the system writes one log-transformed SVG plot per metric under `plots/<app_label>/<metric>_log.svg`

#### Scenario: Zero appears in log-transformed plot
- **WHEN** a metric value is zero
- **THEN** the log-transformed plot shows that point at y = -1

### Requirement: Breakdown CSV contains per-tag snapshot metrics
The system SHALL generate one breakdown CSV per collected snapshot under `breakdowns/<snapshot_label>/<app_label>.csv` containing tag-level Size, Rss, Pss, Referenced, Shared, Private, Swap, SwapPss, and Anonymous totals. The header SHALL be `tag,size_kb,rss_kb,pss_kb,referenced_kb,shared_kb,private_kb,swap_kb,swap_pss_kb,anonymous_kb`, matching the metric column names used by `summary.csv`. Breakdown data rows SHALL be sorted by `size_kb` descending, with tag ascending as a deterministic tie-breaker.

#### Scenario: Snapshot has tag breakdowns
- **WHEN** a raw smaps snapshot is parsed
- **THEN** the system writes a breakdown CSV with header `tag,size_kb,rss_kb,pss_kb,referenced_kb,shared_kb,private_kb,swap_kb,swap_pss_kb,anonymous_kb`

#### Scenario: Breakdown rows are sorted by size
- **WHEN** a breakdown CSV is written for multiple tags
- **THEN** the data rows are ordered by `size_kb` from largest to smallest

#### Scenario: Multiple snapshots exist for one app label
- **WHEN** multiple snapshots exist for the same app label
- **THEN** the system writes a separate breakdown CSV for each snapshot under that snapshot label directory