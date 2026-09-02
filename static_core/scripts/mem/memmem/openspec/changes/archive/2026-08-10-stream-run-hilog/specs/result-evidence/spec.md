## MODIFIED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that copies the input `flow.json`, stores app metadata at `app_metadata.json`, stores snapshot metadata at `snapshots/metadata.json`, stores screenshot metadata at `screenshots/metadata.json`, stores raw smaps snapshots under `snapshots/<snapshot_label>/<app_label>.smaps`, stores screenshot artifacts under `screenshots/<screenshot_label>.png`, stores the run-wide hilog artifact at `logs/hilog.log` when logs are enabled, and stores generated memory trend plots under `plots/<app_label>/`.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a flow path and output directory
- **THEN** the system creates the output directory, `snapshots/`, `screenshots/`, `breakdowns/`, and a copy of `flow.json` before device actions

#### Scenario: Benchmark output is initialized with logs enabled
- **WHEN** the benchmark starts with log collection enabled
- **THEN** the system creates `logs/` before device actions

#### Scenario: Remote output is initialized
- **WHEN** the benchmark creates remote output storage with log collection enabled
- **THEN** the remote output directory name under `/data/local/tmp` matches the local output directory name and contains remote snapshot, screenshot, and logs directories

#### Scenario: App metadata is written
- **WHEN** app-flow command execution finishes or fails after launching any apps and before artifact receive
- **THEN** the system writes `app_metadata.json` containing an `apps` array of PID, label, bundle, and ability records in app launch order

#### Scenario: Snapshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any snapshots and before artifact receive
- **THEN** the system writes `snapshots/metadata.json` containing an `artifacts` array of snapshot labels and timestamps in snapshot command order

#### Scenario: Screenshot metadata is written
- **WHEN** app-flow command execution finishes or fails after staging any screenshots and before artifact receive
- **THEN** the system writes `screenshots/metadata.json` containing an `artifacts` array of screenshot labels and timestamps in screenshot command order

#### Scenario: Raw snapshot is received
- **WHEN** a pending device-local snapshot artifact is received for an app label and snapshot label
- **THEN** the system stores the received smaps file at `snapshots/<snapshot_label>/<app_label>.smaps`

#### Scenario: Raw screenshot is received
- **WHEN** a pending device-local screenshot artifact is received for a screenshot label
- **THEN** the system stores the received PNG file at `screenshots/<screenshot_label>.png`

#### Scenario: Run-wide log is received
- **WHEN** the pending run-wide device-local hilog artifact is received
- **THEN** the system stores the received log file at `logs/hilog.log`

#### Scenario: Plot artifact is written
- **WHEN** report generation produces plots for an app label
- **THEN** the system stores generated SVG plot artifacts under `plots/<app_label>/`

### Requirement: Result paths use label metadata as storage identity
The system SHALL use snapshot-label, screenshot-label, app-label, metric-name, and fixed run-wide log-name keyed paths so repeated bundle/ability flows can be represented even when they resolve to the same PID, generated plots can be associated with one app label and one metric, and the run-wide hilog artifact has stable identity.

#### Scenario: Unique labels are stored
- **WHEN** two tracked app-flow entries have different labels
- **THEN** their snapshots are stored under distinct `snapshots/<snapshot_label>/<app_label>.smaps` paths

#### Scenario: Same PID is stored for multiple labels
- **WHEN** two tracked app-flow entries resolve to the same PID with different labels
- **THEN** their metadata records preserve the shared PID under separate app labels

#### Scenario: Screenshot label path is generated
- **WHEN** a screenshot command has label `after_login`
- **THEN** its screenshot artifact path is `screenshots/after_login.png`

#### Scenario: Plot path is generated
- **WHEN** a plot is generated for app label `gallery` and metric `rss`
- **THEN** its linear plot path is `plots/gallery/rss.svg` and its log-transformed plot path is `plots/gallery/rss_log.svg`

#### Scenario: Run-wide log path is generated
- **WHEN** log collection is enabled
- **THEN** its local artifact path is `logs/hilog.log` and its remote artifact path is `<remote_out_dir>/logs/hilog.log`

#### Scenario: Label-derived path components are generated
- **WHEN** the system creates paths containing app labels, snapshot labels, or screenshot labels
- **THEN** it uses labels already validated as filename-safe path components

### Requirement: Pending artifacts are received before cleanup
The system SHALL track pending snapshot, screenshot, and enabled log artifacts in the benchmark execution context as a flat list of remote base, local base, and relative artifact path records. It SHALL receive all pending device-local artifacts into the output directory before removing the remote run directory. Artifact receive SHALL attempt every pending artifact before reporting receive failure.

#### Scenario: Pending log artifact is received successfully
- **WHEN** a pending run-wide log artifact exists on the device
- **THEN** the system receives it into `logs/hilog.log`

#### Scenario: Pending screenshot artifact is received successfully
- **WHEN** a pending screenshot artifact exists on the device
- **THEN** the system receives it into `screenshots/<screenshot_label>.png`

#### Scenario: Pending snapshot, screenshot, and log artifacts are received together
- **WHEN** pending snapshot, screenshot, and run-wide log artifacts exist on the device
- **THEN** the system receives all pending artifacts before removing the remote run directory

#### Scenario: One artifact fails during receive
- **WHEN** one pending artifact cannot be received from the device
- **THEN** the system still attempts every other pending artifact before failing the benchmark

#### Scenario: Log collection is disabled
- **WHEN** log collection is disabled
- **THEN** the system skips pending log artifact tracking and receives other pending artifacts

#### Scenario: Remote cleanup fails
- **WHEN** the remote run directory cannot be removed after artifact receive
- **THEN** the system fails the benchmark
