## Why

The current benchmark model treats PID as the storage and runtime identity for launched apps. That assumption breaks workflows such as `app1 -> app2 -> app1`, where starting the same bundle/ability can foreground an existing process instead of creating a new PID.

## What Changes

- **BREAKING**: App `label` becomes the unique logical identity for an `AppFlow`; duplicate app labels are rejected during flow validation.
- **BREAKING**: Snapshot command labels become globally unique within a flow; duplicate snapshot labels are rejected during flow validation.
- **BREAKING**: Raw snapshot evidence is stored under `snapshots/<snapshot_label>/<app_label>.smaps` instead of `snapshots/pid-<pid>/<snapshot_label>-<timestamp>.smaps`.
- **BREAKING**: Per-app metadata moves from `snapshots/pid-<pid>/metadata.json` to a single root `app_metadata.json` file.
- Snapshot timestamps and artifact names move from smaps filenames into a root `snapshot_metadata.json` file.
- PID remains recorded as runtime metadata and as the smaps capture target, but it is no longer used as the storage identity.
- Launching a bundle/ability whose process is already running is valid; the resolved current PID is associated with the current unique app label.
- PID resolution fails when `pidof` returns zero or multiple parseable PIDs for the bundle.
- Report generation reads root metadata, processes snapshots in metadata order, and writes mirrored breakdowns under `breakdowns/<snapshot_label>/<app_label>.csv`.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: App and snapshot label uniqueness, PID runtime semantics, repeated bundle/ability launch behavior, and snapshot metadata recording change.
- `result-evidence`: Output layout, artifact identity, metadata files, snapshot filenames, breakdown layout, and report discovery change from PID-keyed/timestamped to metadata-ordered label-keyed evidence.
- `device-hdc`: PID resolution no longer excludes already tracked PIDs and returns the current PID for a bundle only when exactly one PID is resolved.

## Impact

- `src/schema.py`: add global validation for unique app labels and unique snapshot labels.
- `src/device.py`: change PID resolution API from `resolve_pid(bundle, excluded_pids)` to current-PID resolution for a bundle with zero/multiple PID failures.
- `src/device.py`: keep `timestamp()` for snapshot metadata, no longer for filenames.
- `src/device.py`: remove `LaunchedProcess` and use `ProcessMetadata` for runtime tracking.
- `src/runner.py`: accumulate app metadata and snapshot metadata in memory and write `app_metadata.json` and `snapshot_metadata.json` once after command execution, before artifact receive/report generation.
- `src/commands.py`: snapshot capture writes to `snapshots/<snapshot_label>/<app_label>.smaps`, records pending artifacts, and records one timestamp per snapshot command.
- `src/result.py`: replace PID-keyed path APIs/parsers with metadata-file and snapshot/app-label path APIs.
- `src/report.py`: read `app_metadata.json` and `snapshot_metadata.json`, iterate in metadata order, skip missing artifacts, keep timestamp in `summary.csv`, and write mirrored breakdowns.
- `README.md`: update flow/output documentation.
- Tests covering schema, result layout, report generation, device PID resolution, runner behavior, and snapshot capture need updates.
