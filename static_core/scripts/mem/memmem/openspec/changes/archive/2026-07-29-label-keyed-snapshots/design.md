## Context

The current execution model uses PID as the identity for a launched app. The runner launches each `AppFlow`, resolves a PID while excluding already tracked PIDs, stores metadata under `snapshots/pid-<pid>/`, and writes each smaps artifact as `<snapshot_label>-<timestamp>.smaps`.

That model assumes each `AppFlow` launch creates a distinct process. OpenHarmony app lifecycle behavior does not guarantee that: launching the same bundle/ability can foreground an existing app process. The benchmark needs to support flows such as `app1 -> app2 -> app1`, where the same runtime PID can legitimately be associated with multiple logical app-flow steps.

HDC is not the source of these lifecycle semantics. HDC forwards shell commands to the device shell; `aa start` and `pidof` behavior are provided by device-side tools and app framework services.

## Goals / Non-Goals

**Goals:**
- Make app `label` the unique logical identity for each `AppFlow`.
- Require globally unique snapshot labels across the flow.
- Store raw evidence in deterministic metadata-ordered paths under `snapshots/<snapshot_label>/<app_label>.smaps`.
- Store app metadata in root `app_metadata.json` and snapshot metadata in root `snapshot_metadata.json`.
- Record one timestamp per logical snapshot command in snapshot metadata.
- Allow multiple app labels to resolve to the same PID when the device foregrounds an existing process.
- Preserve PID in app metadata and reports as runtime evidence.
- Keep snapshot semantics unchanged: a snapshot command captures all currently tracked app-flow labels whose stored PID still exists.

**Non-Goals:**
- Do not introduce process rediscovery after launch if a stored PID dies and the app relaunches outside framework control.
- Do not change HDC shell transport behavior.
- Do not support duplicate app labels or duplicate snapshot labels.
- Do not migrate existing result directories automatically.
- Do not make `ResultStore` responsible for file I/O; it remains a side-effect-free path/layout service.

## Decisions

### Use app label as logical identity and metadata arrays as ordering source

`AppFlow.label` becomes the stable logical identifier for app metadata and raw snapshots. This makes repeated bundle/ability flows representable even if device runtime PID is reused.

Runtime tracking should use `ProcessMetadata` directly and remove `LaunchedProcess`, because both structures have the same fields: `pid`, `label`, `bundle`, and `ability`.

Affected APIs and structures:
- `AppFlow.label`: changes from display metadata to required unique identifier.
- `LaunchedProcess`: removed.
- `ProcessMetadata`: used for runtime tracking and persisted app metadata.
- New `AppMetadata` model: `{ "apps": [ProcessMetadata, ...] }`.
- New `SnapshotMetadata` model: `{ "label": str, "timestamp": str, "artifacts": [SnapshotRelativeParts, ...] }`.
- New `SnapshotMetadataFile` model: `{ "snapshots": [SnapshotMetadata, ...] }`.
- `SnapshotRelativeParts` remains `tuple[str, str]` to describe snapshot-relative artifact parts: the first part is `<snapshot_label>` and the second part is `<app_label>.smaps`.

Rejected alternative: keep PID-keyed directories and add sequence numbers. That preserves old behavior but keeps the wrong conceptual identity and makes repeated app flows harder to understand.

### Require global uniqueness for app labels and snapshot labels

The schema should reject duplicate app labels and duplicate snapshot labels across all commands in the flow. This avoids path collisions and keeps flow JSON easy to reason about.

Affected validation flow:
- `Flow.model_validate(...)` should validate after parsing all `AppFlow` entries.
- App labels are collected from `flow[*].label` and must be unique.
- Snapshot labels are collected from every `snapshot` command payload across all app flows and must be unique.
- Existing safe-label regex validation remains unchanged.

Rejected alternative: prefix snapshot filenames with app label and only require per-app uniqueness. Because each snapshot command captures all currently tracked labels, repeated snapshot labels later in the flow can still overwrite an earlier artifact for an already tracked app.

### Resolve current PID, not new PID

The device API should resolve the current PID for a bundle without excluding already tracked PIDs. Reusing an existing PID is valid and expected.

Affected APIs and flows:
- `Device.resolve_pid(bundle, excluded_pids)` becomes `Device.resolve_pid(bundle)` or equivalent.
- Runner no longer computes `{process.pid for process in tracked_processes}`.
- Failure means no PID can be parsed from `pidof` output after a successful launch, or more than one PID is parsed.
- If multiple PIDs are printed, the benchmark fails rather than guessing which process represents the app flow.

### Store snapshot timestamp as metadata, not artifact identity

Snapshot smaps filenames should not contain timestamps. For each `snapshot` command, the command layer should call `Device.timestamp()` exactly once before capturing any app smaps and append `{ "label": <snapshot_label>, "timestamp": <timestamp>, "artifacts": [] }` to in-memory snapshot metadata. Each successful smaps capture appends its snapshot-relative artifact parts to that snapshot's `artifacts` list.

The timestamp represents the logical snapshot command time, not each individual app capture time. Metadata, including artifact names, is accumulated in memory and written once after command execution, before artifact receive and report generation.

Affected APIs and structures:
- `Device.timestamp()` remains required but is used once per snapshot command.
- `ProcessSnapshot.timestamp` should be removed if `ProcessSnapshot` remains; reports can avoid `ProcessSnapshot` and iterate metadata directly.
- `_parse_snapshot_filename(...)` should become unnecessary for report ordering.
- `ResultStore.local_process_snapshot_path(..., timestamp)` and `remote_process_snapshot_path(..., timestamp)` are replaced with snapshot/app-label APIs.

### Use snapshot-keyed artifact directories

Raw snapshots are stored under snapshot-label directories with one app-label file per captured app:

```text
snapshots/<snapshot_label>/<app_label>.smaps
```

Breakdowns mirror the same structure:

```text
breakdowns/<snapshot_label>/<app_label>.csv
```

Remote snapshot artifacts mirror the local snapshot layout under the remote run directory:

```text
/data/local/tmp/memmem-<run_id>/snapshots/<snapshot_label>/<app_label>.smaps
```

Affected `ResultStore` APIs:
- Add `local_app_metadata_path()`.
- Add `local_snapshot_metadata_path()`.
- Add `local_snapshot_dir(snapshot_label)`.
- Add `local_snapshot_path(snapshot_label, app_label)`.
- Add `remote_snapshot_dir(snapshot_label)`.
- Add `remote_snapshot_path(snapshot_label, app_label)`.
- Add `local_breakdown_dir(snapshot_label)`.
- Add `local_breakdown_path(snapshot_label, app_label)`.
- Remove PID-keyed directory discovery and timestamped filename parsing APIs.

### Report from metadata, not path parsing

Report generation should read `app_metadata.json` and `snapshot_metadata.json`, then iterate snapshots in snapshot metadata array order and apps in app metadata array order. It should check expected paths instead of parsing labels from paths.

Affected report flow:
- Summary rows keep `pid`, `app_label`, `snapshot_label`, and `timestamp`.
- Timestamp comes from `snapshot_metadata.json`.
- Missing expected smaps files are skipped; this represents dead PIDs skipped during snapshot capture, failed-run remnants, or snapshots with no live tracked apps.
- Snapshot artifact names are also present in `snapshot_metadata.json` for traceability, but reports still derive expected paths from snapshot and app metadata order.
- Breakdown CSVs are written to `breakdowns/<snapshot_label>/<app_label>.csv`.

## Risks / Trade-offs

- Existing output directories cannot be parsed by the new report generator → Treat as a breaking layout change and do not promise backward compatibility.
- Same PID can appear under multiple app labels and produce duplicate smaps captures for a single snapshot → This is intended evidence for logical flow steps; metadata preserves the shared PID.
- If `pidof` prints multiple PIDs for a bundle, the benchmark cannot safely choose one → Fail PID resolution and surface the ambiguity to the user.
- Metadata files are written after command execution and before artifact receive/report generation → If benchmark fails earlier, metadata files may not exist; this is acceptable for a failed benchmark.
- A snapshot can be recorded even when all tracked PIDs are dead → Reports skip missing artifacts and emit no rows for that snapshot.
