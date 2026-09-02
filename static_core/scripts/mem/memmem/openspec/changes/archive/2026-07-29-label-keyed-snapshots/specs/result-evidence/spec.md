## MODIFIED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that copies the input `flow.json`, stores app metadata at `app_metadata.json`, stores snapshot metadata at `snapshot_metadata.json`, and stores raw smaps snapshots under `snapshots/<snapshot_label>/<app_label>.smaps`.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a flow path and output directory
- **THEN** the system creates the output directory, `snapshots/`, and a copy of `flow.json` before device actions

#### Scenario: App metadata is written
- **WHEN** app-flow command execution completes successfully before artifact receive
- **THEN** the system writes `app_metadata.json` containing an `apps` array of PID, label, bundle, and ability records in app launch order

#### Scenario: Snapshot metadata is written
- **WHEN** app-flow command execution completes successfully before artifact receive
- **THEN** the system writes `snapshot_metadata.json` containing a `snapshots` array of snapshot labels, timestamps, and snapshot-relative artifact names in snapshot command order

#### Scenario: Raw snapshot is received
- **WHEN** a pending device-local snapshot artifact is received for an app label and snapshot label
- **THEN** the system stores the received smaps file at `snapshots/<snapshot_label>/<app_label>.smaps`

### Requirement: Result paths use label metadata as storage identity
The system SHALL use snapshot-label and app-label keyed paths so repeated bundle/ability flows can be represented even when they resolve to the same PID.

#### Scenario: Unique labels are stored
- **WHEN** two tracked app-flow entries have different labels
- **THEN** their snapshots are stored under distinct `snapshots/<snapshot_label>/<app_label>.smaps` paths

#### Scenario: Same PID is stored for multiple labels
- **WHEN** two tracked app-flow entries resolve to the same PID with different labels
- **THEN** their metadata records preserve the shared PID under separate app labels

#### Scenario: Label-derived path components are generated
- **WHEN** the system creates paths containing app labels or snapshot labels
- **THEN** it uses labels already validated as filename-safe path components

### Requirement: Pending snapshot artifacts are received before analysis
The system SHALL receive every pending device-local snapshot artifact into the output directory before generating reports.

#### Scenario: Pending artifact is received successfully
- **WHEN** a pending snapshot artifact exists on the device
- **THEN** the system receives it into `snapshots/<snapshot_label>/<app_label>.smaps`

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
The system SHALL generate `summary.csv` from raw snapshots with one row per received snapshot containing PID, app label, snapshot label, timestamp, Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous totals. Snapshot row ordering SHALL follow `snapshot_metadata.json` array order, then `app_metadata.json` array order.

#### Scenario: Raw snapshots exist
- **WHEN** report generation runs over an output directory with app metadata, snapshot metadata, and raw smaps snapshots
- **THEN** the system parses each existing expected snapshot and writes a corresponding total row to `summary.csv`

#### Scenario: Snapshot metadata exists
- **WHEN** a summary row is generated for a snapshot label
- **THEN** the system uses `snapshot_metadata.json` to include the snapshot timestamp

#### Scenario: App metadata exists
- **WHEN** a summary row is generated for an app label
- **THEN** the system uses `app_metadata.json` to include runtime PID and display metadata

#### Scenario: Expected raw snapshot is missing
- **WHEN** metadata references a snapshot label and app label whose expected smaps file is absent
- **THEN** the system skips that summary row without failing report generation

### Requirement: Breakdown CSV contains per-tag snapshot metrics
The system SHALL generate one breakdown CSV per collected snapshot under `breakdowns/<snapshot_label>/<app_label>.csv` containing tag-level Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous totals.

#### Scenario: Snapshot has tag breakdowns
- **WHEN** a raw smaps snapshot is parsed
- **THEN** the system writes a breakdown CSV with header `tag,Size_total_for_tag,Rss_total_for_tag,Pss_total_for_tag,Referenced_total_for_tag,Shared_total_for_tag,Private_total_for_tag,Swap_total_for_tag,Anonymous_total_for_tag`

#### Scenario: Multiple snapshots exist for one app label
- **WHEN** multiple snapshots exist for the same app label
- **THEN** the system writes a separate breakdown CSV for each snapshot under that snapshot label directory
