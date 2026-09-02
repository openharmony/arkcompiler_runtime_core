## MODIFIED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that copies the input `flow.json`, stores app metadata at `app_metadata.json`, stores snapshot metadata at `snapshot_metadata.json`, stores screenshot metadata at `screenshot_metadata.json`, stores raw smaps snapshots under `snapshots/<snapshot_label>/<app_label>.smaps`, stores screenshot artifacts under `screenshots/<screenshot_label>.png`, and stores AppFlow log artifacts under `logs/<app_label>.log` when logs are enabled.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a flow path and output directory
- **THEN** the system creates the output directory, `snapshots/`, `screenshots/`, `breakdowns/`, and a copy of `flow.json` before device actions

#### Scenario: Benchmark output is initialized with logs enabled
- **WHEN** the benchmark starts with log collection enabled
- **THEN** the system creates `logs/` before device actions

#### Scenario: Remote output is initialized
- **WHEN** the benchmark creates remote output storage
- **THEN** the remote output directory name under `/data/local/tmp` matches the local output directory name and contains remote snapshot and screenshot directories

#### Scenario: App metadata is written
- **WHEN** app-flow command execution finishes or fails after launching any apps and before artifact receive
- **THEN** the system writes `app_metadata.json` containing an `apps` array of PID, label, bundle, and ability records in app launch order

#### Scenario: Snapshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any snapshots and before artifact receive
- **THEN** the system writes `snapshot_metadata.json` containing a `snapshots` array of snapshot labels, timestamps, and snapshot-relative artifact names in snapshot command order

#### Scenario: Screenshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any screenshots and before artifact receive
- **THEN** the system writes `screenshot_metadata.json` containing a `screenshots` array of screenshot labels, timestamps, and screenshot-relative artifact names in screenshot command order

#### Scenario: Raw snapshot is received
- **WHEN** a pending device-local snapshot artifact is received for an app label and snapshot label
- **THEN** the system stores the received smaps file at `snapshots/<snapshot_label>/<app_label>.smaps`

#### Scenario: Raw screenshot is received
- **WHEN** a pending device-local screenshot artifact is received for a screenshot label
- **THEN** the system stores the received PNG file at `screenshots/<screenshot_label>.png`

#### Scenario: Raw log is received
- **WHEN** a pending device-local log artifact is received for an app label
- **THEN** the system stores the received log file at `logs/<app_label>.log`

### Requirement: Result paths use label metadata as storage identity
The system SHALL use snapshot-label, screenshot-label, and app-label keyed paths so repeated bundle/ability flows can be represented even when they resolve to the same PID.

#### Scenario: Unique labels are stored
- **WHEN** two tracked app-flow entries have different labels
- **THEN** their snapshots are stored under distinct `snapshots/<snapshot_label>/<app_label>.smaps` paths

#### Scenario: Same PID is stored for multiple labels
- **WHEN** two tracked app-flow entries resolve to the same PID with different labels
- **THEN** their metadata records preserve the shared PID under separate app labels

#### Scenario: Screenshot label path is generated
- **WHEN** a screenshot command has label `after_login`
- **THEN** its screenshot artifact path is `screenshots/after_login.png`

#### Scenario: Label-derived path components are generated
- **WHEN** the system creates paths containing app labels, snapshot labels, or screenshot labels
- **THEN** it uses labels already validated as filename-safe path components

### Requirement: Pending artifacts are received before cleanup
The system SHALL track pending log artifacts and pending screenshot artifacts in the benchmark execution context using the same relative path representation as snapshot artifacts and receive all pending device-local artifact groups into the output directory before removing the remote run directory. Artifact receive SHALL attempt every pending artifact group before reporting receive failure.

#### Scenario: Pending log artifact is received successfully
- **WHEN** a pending log artifact exists on the device
- **THEN** the system receives it into `logs/<app_label>.log`

#### Scenario: Pending screenshot artifact is received successfully
- **WHEN** a pending screenshot artifact exists on the device
- **THEN** the system receives it into `screenshots/<screenshot_label>.png`

#### Scenario: Pending snapshot, screenshot, and log artifacts are received together
- **WHEN** pending snapshot, screenshot, and log artifacts exist on the device
- **THEN** the system receives all artifact groups before removing the remote run directory

#### Scenario: One artifact group fails during receive
- **WHEN** one artifact group cannot be received from the device
- **THEN** the system still attempts every other pending artifact group before failing the benchmark

#### Scenario: Log collection is disabled
- **WHEN** log collection is disabled
- **THEN** the system skips pending log artifact tracking and receives other pending artifact groups

#### Scenario: Remote cleanup fails
- **WHEN** the remote run directory cannot be removed after artifact receive
- **THEN** the system fails the benchmark
