## Context

Current hilog collection clears the hilogd buffer (`hilog -r`) before each AppFlow and dumps it after that AppFlow (`hilog -x > remote_path`). OpenHarmony sources in `base/hiviewdfx/hilog` confirm:

- `hilog -x` (non-blocking): prints current hilogd buffer contents and exits (`hilogtool/main.cpp:65-66`, `service_controller.cpp:523`).
- Default buffer per type: 256 KiB (`log_buffer.cpp:35`). When full, 5% of the oldest matching-type entries are dropped (`log_buffer.cpp:101-106`).
- `hilog` without `-x` blocks and continuously emits new log entries (`service_controller.cpp:528-530`).

Live-device verification using `HDC_PATH` from `.env` confirmed that `hdc shell hilog` streams continuously until terminated, while `hdc shell hilog -x` exits after a buffered dump.

The better model is a single run-wide device-local stream: `hilog > /data/local/tmp/<out>/logs/hilog.log`, started once and later received through the existing pending-artifact path.

## Goals / Non-Goals

**Goals:**
- One run-wide hilog artifact at `logs/hilog.log`.
- Make `--no-reboot` the default CLI behavior so rebooting is opt-in while log collection remains enabled by default.
- Keep HDC encapsulated behind `Device`.
- Use a mandatory device-local remote path so stream output does not accumulate in host RAM.
- Start stream after health verification, remote output setup, and execution context preparation succeed, before AppFlow launch.
- Stop stream in benchmark finalization regardless of success or failure.
- Aggregate stream stop/receive failures with existing error-combination behavior.
- Remove per-AppFlow `hilog -r` and `hilog -x` operations.

**Non-Goals:**
- `log_metadata.json`.
- Per-AppFlow log files or in-stream framework markers.
- Host-side direct stream redirection.
- Log rotation or size limits.

## Decisions

### Decision 1: Device exposes a blocking `hilog(remote_path)` API

Add a mandatory-path blocking operation:

```python
class Device:
    def hilog(self, path: pathlib.PurePosixPath) -> None: ...
```

Implementation command:

```text
hdc shell "hilog > <path>"
```

**Rationale:** The path is device-local because redirection is part of the remote shell command. Making the argument mandatory prevents accidental stdout capture into host process memory.

### Decision 2: Runner uses `multiprocessing.Process` to run `Device.hilog()`

The runner starts a child process that calls `device.hilog(store.remote_hilog_path())`. The main process retains the child process handle and stops it during finalization.

**Rationale:** A blocking `Device.hilog()` preserves HDC encapsulation and remains easy to understand. `multiprocessing.Process` gives the main runner a killable host-side unit, unlike a thread.

### Decision 3: Keep one fixed pending log artifact

When logs are enabled, the runner records exactly one pending log artifact in `ExecutionContext.pending_artifacts`: `logs/hilog.log`. `_receive_all_pending_artifacts()` continues receiving artifacts using the existing mechanism.

**Rationale:** This avoids new metadata and preserves the current artifact-transfer model while using one flat pending artifact list. The log semantic change is cardinality and timing: one run log instead of one per AppFlow.

### Decision 4: Start order moves after startup validation

New order:

```text
local output init
prepare device and configure hilog
verify device health
remote output init, including remote logs/
prepare execution context, including screen bounds and pending log artifact
start multiprocessing child: device.hilog(remote logs/hilog.log)
execute AppFlows
finalize: stop child, write metadata, receive artifacts, cleanup remote
```

**Rationale:** Because the hilog output path is device-local, the remote logs directory must exist before streaming starts. Health verification and execution context preparation fail immediately before the stream starts, avoiding child-process cleanup on startup validation errors while still capturing AppFlows.

### Decision 5: No markers

Do not inject framework markers into hilog.

**Rationale:** The log should remain raw hilog output. Existing hilog timestamps are sufficient for later analysis.

## Alternatives to assess

### Stopping the multiprocessing child

Start the hilog child as a process-group leader and stop the group with `SIGTERM -> join(timeout=5) -> SIGKILL -> join()`, falling back to `multiprocessing.Process.terminate()` / `kill()` if group signaling is unavailable.

**Rationale:** This keeps cleanup bounded and avoids leaked `hdc shell hilog` descendants when terminating only the Python multiprocessing child would not stop the HDC subprocess. The simpler `terminate() -> join(timeout=5)` leaves the benchmark with ambiguous cleanup state when descendants remain alive.

### Process testing strategy

Use real `multiprocessing.Process` in tests instead of patching the runner with a process factory.

**Rationale:** This tests the production lifecycle directly and avoids adding an artificial dependency-injection seam only for tests. Tests should assert observable artifacts and process cleanup behavior rather than relying on shared fake-device memory from the child process.

## Affected APIs, Structs, and Signatures

### Added API

```python
# src/device.py
class Device:
    def hilog(self, path: pathlib.PurePosixPath) -> None: ...
```

### Removed APIs

```python
# src/device.py
class Device:
    def clear_hilog(self) -> None: ...
    def dump_hilog(self, remote_path: pathlib.PurePosixPath) -> None: ...
```

### Unchanged API with changed semantics

```python
# src/options.py
@dataclasses.dataclass(frozen=True)
class BenchmarkOptions:
    flow_path: pathlib.Path
    out_dir: pathlib.Path
    reboot: bool = False  # default changes to no reboot
    logs: bool = True     # remains default-enabled; True means one run-wide device-local hilog stream
```

### Existing struct kept

```python
# src/commands.py
@dataclasses.dataclass
class PendingArtifact:
    remote_base: pathlib.PurePosixPath
    local_base: pathlib.Path
    artifact: RelativeParts

class ExecutionContext:
    device: Device
    store: ResultStore
    flow: Flow
    apps: list[AppMetadata]
    pending_artifacts: list[PendingArtifact]
    artifact_metadata: dict[pathlib.Path, list[ArtifactMetadata]]
    child_processes: list[multiprocessing.Process]
    screen_bounds: ScreenBounds
```

### ResultStore helpers

```python
# src/result.py
class ResultStore:
    def local_logs_dir(self) -> pathlib.Path: ...          # unchanged
    def remote_logs_dir(self) -> pathlib.PurePosixPath: ... # unchanged

    def local_hilog_path(self) -> pathlib.Path: ...        # NEW: logs/hilog.log
    def remote_hilog_path(self) -> pathlib.PurePosixPath: ... # NEW: remote logs/hilog.log
    def hilog_relative_parts(self) -> RelativeParts: ...   # NEW: ["hilog.log"]
    def local_artifact_metadata_path(self, path: pathlib.Path) -> pathlib.Path: ... # NEW: <path>/metadata.json

    def local_log_path(self, app_label: str) -> pathlib.Path: ... # REMOVED or unused
    def remote_log_path(self, app_label: str) -> pathlib.PurePosixPath: ... # REMOVED or unused
    def log_relative_parts(self, app_label: str) -> RelativeParts: ... # REMOVED or unused
```

### Runner internal helpers

```python
# src/runner.py

def _pre_flow(options: BenchmarkOptions, store: ResultStore, device: Device) -> ExecutionContext: ...
def _flow(context: ExecutionContext) -> None: ...
def _post_flow(store: ResultStore, context: ExecutionContext) -> None: ...
def _start_hilog_process(device: Device, remote_path: pathlib.PurePosixPath) -> multiprocessing.Process: ...
def _stop_process(process: multiprocessing.Process, timeout_seconds: float = 5) -> None: ...
```

### Fake device changes

```python
# test/mock/device.py
class FakeDevice:
    fail_hilog: bool = False              # NEW
    hilog_paths: list[pathlib.PurePosixPath] # NEW
    default_log_content: str              # existing, reused

    def hilog(self, path: pathlib.PurePosixPath) -> None: ... # NEW, blocking-compatible fake

    fail_clear_hilog: bool                # REMOVED
    fail_dump_hilog: bool                 # REMOVED
    hilog_cleared_count: int              # REMOVED
```

### Metadata structs

No `log_metadata.json`.

Metadata models:

```python
RelativeParts = list[str]
AppMetadata
AppMetadataFile
ArtifactMetadata
ArtifactMetadataFile
```

Snapshot and screenshot metadata use `snapshots/metadata.json` and `screenshots/metadata.json` with the shared `ArtifactMetadataFile` shape.

## Flow Changes

### Current flow

```text
local init
prepare device/config hilog
health
remote init
screen bounds
for each AppFlow:
  hilog -r
  app commands
  hilog -x > remote logs/<app_label>.log
metadata
receive snapshots/screenshots/logs
cleanup
```

### New flow

```text
local init
prepare device/config hilog
health
remote init, including remote logs/
prepare execution context, including screen bounds and pending log artifact ["hilog.log"] when logs are enabled
start child process: Device.hilog(remote logs/hilog.log)
for each AppFlow:
  app commands
metadata
stop child process
receive snapshots/screenshots/logs/hilog.log
cleanup
```

## Risks / Trade-offs

- **Remote file grows without rotation:** Long flows may consume device storage. Mitigation: benchmark flows are controlled; rotation can be a follow-up.
- **Start is later than host-direct streaming:** Remote dirs, health verification, and execution context setup must complete before streaming. Mitigation: this still captures all AppFlows and avoids child-process cleanup on startup validation errors.
- **Multiprocessing child may not share fake state:** Tests need a process seam or focused helper tests. Mitigation: isolate process lifecycle helpers.
- **Stop may race with file flush:** Killing `hdc shell hilog` could truncate the last few log lines. Mitigation: terminate and wait before kill fallback.

## Open Questions

None.
