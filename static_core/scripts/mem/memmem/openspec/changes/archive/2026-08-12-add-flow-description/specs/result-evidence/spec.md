## MODIFIED Requirements

### Requirement: Output directory preserves raw evidence
The system SHALL create an output directory that stores canonical validated flow JSON at `flow.json`, stores app metadata at `app_metadata.json`, stores snapshot metadata at `snapshots/metadata.json`, stores screenshot metadata at `screenshots/metadata.json`, stores raw smaps snapshots under `snapshots/<snapshot_label>/<app_label>.smaps`, stores screenshot artifacts under `screenshots/<screenshot_label>.png`, stores the run-wide hilog artifact at `logs/hilog.log` when logs are enabled, and stores generated memory trend plots under `plots/<app_label>/`.

#### Scenario: Benchmark output is initialized
- **WHEN** the benchmark starts with a validated flow and output directory
- **THEN** the system creates the output directory, `snapshots/`, `screenshots/`, `breakdowns/`, and writes `flow.json` as canonical validated flow JSON before device actions

#### Scenario: Benchmark output preserves flow description
- **WHEN** the benchmark starts with a validated flow containing `"$desc"` metadata
- **THEN** output `flow.json` contains the same `"$desc"` metadata in the canonical validated flow JSON

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

#### Scenario: CLI input flow is canonicalized
- **WHEN** benchmark execution is started from `python run.py --flow flow.json`
- **THEN** output `flow.json` contains the canonical validated flow model and need not be byte-for-byte identical to the input file

#### Scenario: Programmatic flow is persisted
- **WHEN** benchmark execution is started through the public programmatic API
- **THEN** output `flow.json` contains the canonical validated flow model passed to execution
