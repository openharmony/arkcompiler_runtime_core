## MODIFIED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that copies the input `flow.json`, stores app metadata at `app_metadata.json`, stores snapshot metadata at `snapshot_metadata.json`, stores raw smaps snapshots under `snapshots/<snapshot_label>/<app_label>.smaps`, and stores AppFlow log artifacts under `logs/<app_label>.log` when logs are enabled.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a flow path and output directory
- **THEN** the system creates the output directory, `snapshots/`, `breakdowns/`, and a copy of `flow.json` before device actions

#### Scenario: Benchmark output is initialized with logs enabled
- **WHEN** the benchmark starts with log collection enabled
- **THEN** the system creates `logs/` before device actions

#### Scenario: Remote output is initialized
- **WHEN** the benchmark creates remote output storage
- **THEN** the remote output directory name under `/data/local/tmp` matches the local output directory name

#### Scenario: App metadata is written
- **WHEN** app-flow command execution finishes or fails after launching any apps and before artifact receive
- **THEN** the system writes `app_metadata.json` containing an `apps` array of PID, label, bundle, and ability records in app launch order

#### Scenario: Snapshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any snapshots and before artifact receive
- **THEN** the system writes `snapshot_metadata.json` containing a `snapshots` array of snapshot labels, timestamps, and snapshot-relative artifact names in snapshot command order

#### Scenario: Raw snapshot is received
- **WHEN** a pending device-local snapshot artifact is received for an app label and snapshot label
- **THEN** the system stores the received smaps file at `snapshots/<snapshot_label>/<app_label>.smaps`

#### Scenario: Raw log is received
- **WHEN** a pending device-local log artifact is received for an app label
- **THEN** the system stores the received log file at `logs/<app_label>.log`

### Requirement: Breakdown CSV contains per-tag snapshot metrics
The system SHALL generate one breakdown CSV per collected snapshot under `breakdowns/<snapshot_label>/<app_label>.csv` containing tag-level Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous totals. Breakdown data rows SHALL be sorted by `Size_total_for_tag` descending, with tag ascending as a deterministic tie-breaker.

#### Scenario: Snapshot has tag breakdowns
- **WHEN** a raw smaps snapshot is parsed
- **THEN** the system writes a breakdown CSV with header `tag,Size_total_for_tag,Rss_total_for_tag,Pss_total_for_tag,Referenced_total_for_tag,Shared_total_for_tag,Private_total_for_tag,Swap_total_for_tag,Anonymous_total_for_tag`

#### Scenario: Breakdown rows are sorted by size
- **WHEN** a breakdown CSV is written for multiple tags
- **THEN** the data rows are ordered by `Size_total_for_tag` from largest to smallest

#### Scenario: Multiple snapshots exist for one app label
- **WHEN** multiple snapshots exist for the same app label
- **THEN** the system writes a separate breakdown CSV for each snapshot under that snapshot label directory

## ADDED Requirements

### Requirement: Pending artifacts are received before cleanup
The system SHALL track pending log artifacts in the benchmark execution context using the same relative path representation as snapshot artifacts and receive all pending device-local artifact groups into the output directory before removing the remote run directory. Artifact receive SHALL attempt every pending artifact group before reporting receive failure.

#### Scenario: Pending log artifact is received successfully
- **WHEN** a pending log artifact exists on the device
- **THEN** the system receives it into `logs/<app_label>.log`

#### Scenario: Pending snapshot and log artifacts are received together
- **WHEN** pending snapshot and log artifacts exist on the device
- **THEN** the system receives both artifact groups before removing the remote run directory

#### Scenario: One artifact group fails during receive
- **WHEN** one pending artifact group cannot be received from the device
- **THEN** the system still attempts every other pending artifact group before failing the benchmark

#### Scenario: Log collection is disabled
- **WHEN** log collection is disabled
- **THEN** the system skips pending log artifact tracking and receives only other pending artifact groups
