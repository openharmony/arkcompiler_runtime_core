## ADDED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that copies the input `flow.json` and stores raw smaps snapshots under PID-keyed directories.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a flow path and output directory
- **THEN** the system creates the output directory, `snapshots/`, and a copy of `flow.json` before device actions

#### Scenario: Process metadata is written
- **WHEN** a launched process PID is stored
- **THEN** the system writes `snapshots/pid-<pid>/metadata.json` containing PID, label, bundle, and ability

#### Scenario: Raw snapshot is received
- **WHEN** a pending device-local snapshot artifact is received for a PID and snapshot label
- **THEN** the system stores the received smaps file at `snapshots/pid-<pid>/<snapshot_label>-<timestamp>.smaps`

### Requirement: Result paths use PID as storage identity
The system SHALL use PID-keyed snapshot directories so duplicate labels and duplicate bundle/ability pairs do not collide in raw evidence storage.

#### Scenario: Duplicate labels are stored
- **WHEN** two tracked processes have the same label and different PIDs
- **THEN** their metadata and snapshots are stored under separate `pid-<pid>` directories

#### Scenario: Label-derived filenames are generated
- **WHEN** the system creates derived filenames containing labels or snapshot labels
- **THEN** it uses labels already validated as filename-safe path components

### Requirement: Pending snapshot artifacts are received before analysis
The system SHALL receive every pending device-local snapshot artifact into the output directory before generating reports.

#### Scenario: Pending artifact is received successfully
- **WHEN** a pending snapshot artifact exists on the device
- **THEN** the system receives it into the corresponding PID-keyed snapshot directory

#### Scenario: Pending artifact is missing during receive
- **WHEN** a pending snapshot artifact cannot be received from the device
- **THEN** the system fails the benchmark and does not generate reports

#### Scenario: All pending artifacts are received
- **WHEN** every pending snapshot artifact has been received successfully
- **THEN** the system removes the remote run directory from the device

#### Scenario: Remote cleanup fails
- **WHEN** the remote run directory cannot be removed after successful artifact receive
- **THEN** the system fails the benchmark

### Requirement: Summary CSV contains total snapshot metrics
The system SHALL generate `summary.csv` from raw snapshots with one row per received snapshot containing PID, app label, snapshot label, timestamp, Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous totals.

#### Scenario: Raw snapshots exist
- **WHEN** report generation runs over an output directory with raw smaps snapshots
- **THEN** the system parses each snapshot and writes a corresponding total row to `summary.csv`

#### Scenario: Snapshot metadata exists
- **WHEN** a summary row is generated for a PID
- **THEN** the system uses that PID's `metadata.json` to include display metadata such as app label

### Requirement: Breakdown CSV contains per-tag snapshot metrics
The system SHALL generate one breakdown CSV per collected snapshot under `breakdowns/` containing tag-level Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous totals.

#### Scenario: Snapshot has tag breakdowns
- **WHEN** a raw smaps snapshot is parsed
- **THEN** the system writes a breakdown CSV with header `tag,Size_total_for_tag,Rss_total_for_tag,Pss_total_for_tag,Referenced_total_for_tag,Shared_total_for_tag,Private_total_for_tag,Swap_total_for_tag,Anonymous_total_for_tag`

#### Scenario: Multiple snapshots exist for one PID
- **WHEN** multiple snapshots exist for the same PID
- **THEN** the system writes a separate breakdown CSV for each snapshot
