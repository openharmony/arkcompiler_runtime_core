## 1. Schema Validation

- [x] 1.1 Add `Flow`-level validation that rejects duplicate `AppFlow.label` values.
- [x] 1.2 Add `Flow`-level validation that rejects duplicate `snapshot` command payload labels across the whole flow.
- [x] 1.3 Add schema tests for duplicate app labels, duplicate snapshot labels, and valid repeated bundle/ability entries with unique labels.

## 2. Device PID Resolution

- [x] 2.1 Change `Device.resolve_pid` to accept only `bundle` and remove excluded-PID filtering.
- [x] 2.2 Make PID resolution fail when `pidof` returns zero parseable PIDs.
- [x] 2.3 Make PID resolution fail when `pidof` returns more than one parseable PID.
- [x] 2.4 Keep `Device.timestamp()` and use it for snapshot metadata timestamps, not filenames.
- [x] 2.5 Update device tests for single PID, zero PID, multiple PID, reused already-tracked PID behavior, and timestamp behavior.

## 3. Metadata Models and Runtime Tracking

- [x] 3.1 Remove `LaunchedProcess`.
- [x] 3.2 Use `ProcessMetadata` for runtime `tracked_processes` entries.
- [x] 3.3 Add an app metadata file model containing `apps: list[ProcessMetadata]`.
- [x] 3.4 Add snapshot metadata models containing snapshot `label`, `timestamp`, and `artifacts`, plus `snapshots: list[...]`.
- [x] 3.5 Update tests for metadata model validation and serialization.

## 4. Result Store Layout

- [x] 4.1 Remove PID-keyed local snapshot directory and metadata path APIs.
- [x] 4.2 Remove PID-keyed remote snapshot directory APIs.
- [x] 4.3 Add root metadata paths for `app_metadata.json` and `snapshot_metadata.json`.
- [x] 4.4 Add local snapshot paths for `snapshots/<snapshot_label>/<app_label>.smaps`.
- [x] 4.5 Add remote snapshot paths mirroring `snapshots/<snapshot_label>/<app_label>.smaps` under the remote run directory.
- [x] 4.6 Add breakdown paths for `breakdowns/<snapshot_label>/<app_label>.csv`.
- [x] 4.7 Remove timestamped snapshot filename parsing and PID directory discovery.
- [x] 4.8 Update result store tests for metadata paths, mirrored snapshot paths, mirrored breakdown paths, and removed PID layout assumptions.

## 5. Runner and Snapshot Flow

- [x] 5.1 Update runner launch flow to call current-PID resolution without an excluded PID set.
- [x] 5.2 Append `ProcessMetadata` to in-memory tracked processes after each successful launch and PID resolution.
- [x] 5.3 Accumulate app metadata in memory and write `app_metadata.json` once after command execution completes successfully and before artifact receive.
- [x] 5.4 Update snapshot command execution to call `Device.timestamp()` once per snapshot command before app smaps capture.
- [x] 5.5 Accumulate snapshot metadata in memory and write `snapshot_metadata.json` once after command execution completes successfully and before artifact receive.
- [x] 5.6 Update snapshot command execution to create remote `snapshots/<snapshot_label>/` directories and capture `<app_label>.smaps` files.
- [x] 5.7 Record pending artifacts as `(snapshot_label, "<app_label>.smaps")` relative parts.
- [x] 5.8 Update artifact receive flow to copy mirrored snapshot-keyed pending artifacts.
- [x] 5.9 Update runner and command tests for `app1 -> app2 -> app1` with shared PID, snapshot metadata timestamps, missing PID skips, and mirrored artifact paths.

## 6. Reports

- [x] 6.1 Update report generation to read `app_metadata.json` and `snapshot_metadata.json` as source of truth.
- [x] 6.2 Iterate snapshots in snapshot metadata array order and apps in app metadata array order.
- [x] 6.3 Keep `timestamp` in `SummaryRow` and `summary.csv`, sourced from snapshot metadata.
- [x] 6.4 Skip missing expected smaps files without failing report generation.
- [x] 6.5 Write breakdown CSVs to `breakdowns/<snapshot_label>/<app_label>.csv`.
- [x] 6.6 Update report tests for metadata-ordered rows, timestamp source, skipped missing artifacts, and mirrored breakdown paths.

## 7. Documentation and Verification

- [x] 7.1 Update README flow-schema documentation to state that app labels and snapshot labels must be globally unique.
- [x] 7.2 Update README output layout documentation to show `app_metadata.json`, `snapshot_metadata.json`, `snapshots/<snapshot_label>/<app_label>.smaps`, and `breakdowns/<snapshot_label>/<app_label>.csv`.
- [x] 7.3 Search the codebase and specs for stale `pid-<pid>`, `excluded_pids`, per-app `metadata.json`, and timestamped snapshot filename references.
- [x] 7.4 Run `openspec validate "label-keyed-snapshots"`.
- [x] 7.5 Run `source ".venv/bin/activate" && make tests_full`.
