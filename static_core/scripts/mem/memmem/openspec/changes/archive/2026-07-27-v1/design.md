## Context

The legacy `memmem` repository demonstrates useful collection and analysis techniques, but its core workflow is obscured by multiple CLI entrypoints, subprocess orchestration between Python modules, large mixed-responsibility runners, and advanced experiment modes integrated into the main path. The clean rewrite starts from a small v1 surface defined by `SPECIFICATION.md`: one CLI, one flow file, PID-based process identity, raw smaps evidence, and CSV reports.

The current `memmem_clean` repository already provides a strict Python 3.11 development environment with `make test`, `make mypy`, and `make tests_full`. The implementation should preserve the specification's emphasis on simplicity and should run `make tests_full` after each phase.

## Goals / Non-Goals

**Goals:**

- Implement a single benchmark path driven by `run.py --flow <flow.json> [--out <outdir>]`.
- Keep `run.py` thin and place framework logic under `src/`.
- Parse `.env` configuration and use `HDC_PATH` for all HDC calls.
- Validate benchmark flow JSON with pydantic, including filename-safe app labels and snapshot labels.
- Launch every `AppFlow`, resolve its PID immediately, and store that PID as runtime identity.
- Execute initial commands: `wait`, `snapshot`, and `key`.
- Capture snapshots quickly on device by redirecting `/proc/<pid>/smaps` into a per-run remote temporary directory.
- Receive all pending snapshot artifacts after all app flows finish, then clean up the remote temporary directory.
- Preserve received raw snapshots under PID-keyed directories.
- Parse smaps into total memory profiles and per-tag breakdowns.
- Generate `summary.csv` and per-snapshot breakdown CSV files.
- Include tests early, starting with full `smaps.py` behavior and fixture coverage.

**Non-Goals:**

- Native allocation tracing.
- Page fingerprinting.
- A/B runtime replacement.
- Gallery-specific workflows.
- Multi-run statistical analysis.
- App-specific scenario validation.
- PNG or Markdown report generation.
- Multiple Python CLI entrypoints.
- Internal orchestration by invoking another Python CLI as a subprocess.
- HDC/device/command mocking tests in the initial pass.

## Decisions

### Decision: PID is runtime identity

The runner stores the PID resolved immediately after launching each `AppFlow`. Labels, bundles, and abilities are metadata only. Duplicate labels and duplicate bundle/ability pairs are allowed. PID resolution excludes PIDs already tracked by the framework so repeated launches of the same bundle can resolve a newly created process when the target platform supports that behavior.

Alternative considered: require unique labels and use labels as identity. This was rejected because benchmark flows may intentionally launch duplicate apps or reuse display labels.

### Decision: Snapshot collection uses device-local capture with deferred transfer

At benchmark start, the runner generates a filename-safe run ID and creates a per-run remote directory:

```text
/data/local/tmp/memmem-<run_id>/
```

The `snapshot` command checks each tracked PID and, for every still-valid PID, obtains a device snapshot timestamp and executes device-local capture:

```text
cat /proc/<pid>/smaps > /data/local/tmp/memmem-<run_id>/pid-<pid>/<snapshot_label>-<snapshot_timestamp>.smaps
```

The command records a pending snapshot artifact as `SnapshotRelativeParts`, such as `("pid-<pid>", "<snapshot_label>-<snapshot_timestamp>.smaps")`. It does not transfer the file immediately.

After all app flows finish, the runner receives every pending artifact into the output directory. If any pending artifact is missing or cannot be received, the benchmark fails. After all artifacts are received successfully, the runner removes the remote temporary directory. Cleanup failure is also a benchmark error.

Alternative considered: direct host capture through `hdc shell cat /proc/<pid>/smaps` stdout. This was rejected because transferring smaps content to the host during the snapshot command widens the timing window and can miss short-lived app states intended to be captured.

### Decision: Keep HDC wrapper minimal

`HdcResult` contains only `returncode`, `stdout`, and `stderr`. `Hdc.run(*args)` executes `<HDC_PATH> <args...>`, and `Hdc.shell(*args)` executes `<HDC_PATH> shell <args...>`. Callers decide whether a non-zero return code is acceptable.

Alternative considered: custom exceptions, stored command args, and `check_returncode()`. This was rejected for v1 to keep the wrapper transparent and avoid imposing policy at the lowest layer.

### Decision: ResultStore owns filesystem layout only

`ResultStore` is a side-effect-free local and remote artifact layout service. It does not create directories, copy `flow.json`, write metadata, execute device operations, validate schema-owned labels, parse smaps, or generate reports.

It is used by:

- `runner.py` to create local/remote directories, copy `flow.json`, write process metadata, and determine final local paths during artifact receive.
- `commands.py` to generate remote snapshot paths and record pending snapshot artifact relative paths during `snapshot` command execution.
- `report.py` to parse local snapshot layout and generate derived CSV outputs.

Alternative considered: combine storage and reporting. This was rejected because raw evidence handling and derived artifact generation should remain separate.

### Decision: Implement smaps analysis first and fully

`src/smaps.py` is implemented in the first phase with real fixtures and tests. It returns:

```python
@dataclasses.dataclass(frozen=True)
class MemProfile:
    size_kb: int
    rss_kb: int
    pss_kb: int
    referenced_kb: int
    shared_kb: int
    private_kb: int
    swap_kb: int
    anonymous_kb: int

@dataclasses.dataclass(frozen=True)
class SmapsSummary:
    total: MemProfile
    breakdown: dict[str, MemProfile]
```

This creates an immediate, device-independent test surface and directly feeds report generation.

Alternative considered: implement only enough parsing to pass one fixture. This was rejected because smaps parsing is independent and should be stable before device orchestration begins.

### Decision: CSV-only reports in v1

Reports generate:

- `summary.csv` with one total row per collected snapshot.
- `breakdowns/<pid>-<app_label>-<snapshot_label>-<timestamp>.csv` with per-tag rows.

PNG and Markdown are deferred.

Alternative considered: implement PNG/Markdown immediately. This was rejected to keep v1 focused and avoid optional plotting/reporting complexity before the evidence model is proven.

### Decision: Tests grow from deterministic modules outward

Initial testing focuses on modules with stable local behavior: smaps parsing, schema/config loading, result storage, report generation, and CLI argument validation. HDC, device, and command mocking tests are deferred.

Alternative considered: mock all device interactions from the start. This was rejected because HDC behavior is better validated after the core local modules and interfaces stabilize.

## API and Layout Sketch

### Smaps analysis

```python
@dataclasses.dataclass(frozen=True)
class MemProfile:
    size_kb: int
    rss_kb: int
    pss_kb: int
    referenced_kb: int
    shared_kb: int
    private_kb: int
    swap_kb: int
    anonymous_kb: int

@dataclasses.dataclass(frozen=True)
class SmapsSummary:
    total: MemProfile
    breakdown: dict[str, MemProfile]

def parse_smaps_text(text: str) -> SmapsSummary:
    ...
```

### Configuration and flow schema

```python
@dataclasses.dataclass(frozen=True)
class Config:
    hdc_path: pathlib.Path

def load_config(env_path: pathlib.Path) -> Config:
    ...

class WaitCommand(pydantic.BaseModel):
    action: Literal["wait"]
    payload: Annotated[int, pydantic.Field(ge=0, strict=True)]

class SnapshotCommand(pydantic.BaseModel):
    action: Literal["snapshot"]
    payload: pydantic.StrictStr

class KeyCommand(pydantic.BaseModel):
    action: Literal["key"]
    payload: dict[str, pydantic.StrictStr]

Command = Annotated[
    WaitCommand | SnapshotCommand | KeyCommand,
    pydantic.Field(discriminator="action"),
]

class AppFlow(pydantic.BaseModel):
    label: str
    bundle: str
    ability: str
    commands: list[Command]

class Flow(pydantic.BaseModel):
    flow: list[AppFlow]
```

`Command` is a discriminated union keyed by `action`. `AppFlow.label` and `snapshot` command payloads must match:

```text
^[A-Za-z0-9_-]+$
```

### HDC wrapper

```python
@dataclasses.dataclass(frozen=True)
class HdcResult:
    returncode: int
    stdout: str
    stderr: str

class Hdc:
    def __init__(self, hdc_path: pathlib.Path) -> None:
        ...

    def run(self, *args: str) -> HdcResult:
        ...

    def shell(self, *args: str) -> HdcResult:
        ...
```

`run(*args)` executes `<HDC_PATH> <args...>`. `shell(*args)` executes `<HDC_PATH> shell <args...>`. The wrapper does not raise for non-zero return codes; callers interpret `HdcResult`.

### Device operations

```python
@dataclasses.dataclass(frozen=True)
class LaunchedProcess:
    pid: int
    label: str
    bundle: str
    ability: str

class Device:
    def __init__(self, hdc: Hdc) -> None:
        ...

    def launch_app(self, bundle: str, ability: str) -> None:
        ...

    def resolve_pid(self, bundle: str, excluded_pids: set[int]) -> int:
        ...

    def pid_exists(self, pid: int) -> bool:
        ...

    def timestamp(self) -> str:
        ...

    def make_dir(self, remote_path: pathlib.PurePosixPath) -> bool:
        ...

    def remove_dir(self, remote_path: pathlib.PurePosixPath) -> bool:
        ...

    def capture_smaps(self, pid: int, remote_path: pathlib.PurePosixPath) -> bool:
        ...

    def send_key(self, payload: dict[str, str]) -> None:
        ...

    def send_file(self, local_path: pathlib.Path, remote_path: pathlib.PurePosixPath) -> HdcResult:
        ...

    def recv_file(self, remote_path: pathlib.PurePosixPath, local_path: pathlib.Path) -> HdcResult:
        ...
```

`timestamp` returns a digit-only device timestamp string. It should use `date +%s%N` when available and fall back to seconds with zero-padded nanoseconds.

`capture_smaps` executes device-local capture:

```text
cat /proc/<pid>/smaps > <remote_path>
```

`send_file` and `recv_file` use legacy HDC transfer patterns:

```text
hdc file send <local> <remote>
hdc file recv <remote> <local>
```

### ResultStore

```python
SnapshotRelativeParts = tuple[str, str]

class ProcessMetadata(pydantic.BaseModel):
    pid: int
    label: str
    bundle: str
    ability: str

@dataclasses.dataclass(frozen=True)
class ProcessSnapshot:
    path: pathlib.Path
    label: str
    timestamp: str

class ResultStore:
    def __init__(
        self,
        local_out_dir: pathlib.Path,
        remote_out_dir: pathlib.PurePosixPath,
    ) -> None:
        ...

    def local_flow_path(self) -> pathlib.Path:
        ...

    def local_snapshots_dir(self) -> pathlib.Path:
        ...

    def local_breakdowns_dir(self) -> pathlib.Path:
        ...

    def local_process_snapshots_dir(self, pid: int) -> pathlib.Path:
        ...

    def local_process_metadata_path(self, pid: int) -> pathlib.Path:
        ...

    def local_process_snapshot_path(self, pid: int, snapshot_label: str, timestamp: str) -> pathlib.Path:
        ...

    def local_process_breakdown_path(self, pid: int, app_label: str, snapshot_label: str, timestamp: str) -> pathlib.Path:
        ...


    def local_parse_pids(self) -> list[int]:
        ...

    def local_parse_process_snapshots(self, pid: int) -> list[ProcessSnapshot]:
        ...

    def remote_out_dir(self) -> pathlib.PurePosixPath:
        ...

    def remote_process_snapshots_dir(self, pid: int) -> pathlib.PurePosixPath:
        ...

    def remote_process_snapshot_path(self, pid: int, snapshot_label: str, timestamp: str) -> pathlib.PurePosixPath:
        ...

```

The runner uses `ResultStore` local and remote APIs to keep host and device artifact paths consistent without string path concatenation.

### Command execution

```python
@dataclasses.dataclass
class ExecutionContext:
    device: Device
    store: ResultStore
    tracked_processes: list[LaunchedProcess]
    snapshots: list[SnapshotRelativeParts]

def execute_command(command: Command, context: ExecutionContext) -> None:
    ...

def execute_wait(payload: int) -> None:
    ...

def execute_snapshot(payload: str, context: ExecutionContext) -> None:
    ...

def execute_key(payload: dict[str, str], context: ExecutionContext) -> None:
    ...
```

`snapshot` appends pending device-relative artifact paths to `ExecutionContext.snapshots` instead of writing local snapshot files immediately.

### Runner flow

```text
load flow
run_id = uuid.uuid4().hex
result_store = ResultStore(out_dir, /data/local/tmp/memmem-<run_id>)
create local out dir and snapshots dir
copy flow.json to result_store.local_flow_path()
create Hdc and Device
device.make_dir(result_store.remote_out_dir())
tracked_processes = []
snapshots = []
context = ExecutionContext(device, result_store, tracked_processes, snapshots)

for app_flow in flow.flow:
    device.launch_app(app_flow.bundle, app_flow.ability)
    pid = device.resolve_pid(
        app_flow.bundle,
        {process.pid for process in tracked_processes},
    )
    process = LaunchedProcess(pid, label, bundle, ability)
    tracked_processes.append(process)
    write process metadata to result_store.local_process_metadata_path(pid)

    for command in app_flow.commands:
        execute_command(command, context)

for snapshot in snapshots:
    remote_path = result_store.remote_out_dir().joinpath(*snapshot)
    local_path = result_store.local_snapshots_dir().joinpath(*snapshot)
    result = device.recv_file(remote_path, local_path)
    if result.returncode != 0:
        fail benchmark

if not device.remove_dir(result_store.remote_out_dir()):
    fail benchmark

generate_reports(result_store)
```

### Output layout

```text
out/
  flow.json
  snapshots/
    pid-<pid>/
      metadata.json
      <snapshot_label>-<snapshot_timestamp>.smaps
  breakdowns/
    <pid>-<app_label>-<snapshot_label>-<snapshot_timestamp>.csv
  summary.csv
```

### Remote layout

```text
/data/local/tmp/memmem-<run_id>/
  pid-<pid>/
    <snapshot_label>-<snapshot_timestamp>.smaps
```

## Risks / Trade-offs

- HDC command details may differ across targets → Reuse exact command patterns from the legacy `~/memmem` implementation where possible.
- PID reuse can theoretically confuse long-running flows → v1 treats stored PID as identity and checks `/proc/<pid>` existence only; more robust process identity checks can be added later if needed.
- Label-derived filenames can become unsafe if schema validation is bypassed → validate app labels and snapshot labels before device actions and keep PID in every derived breakdown filename.
- Remote snapshot artifacts may disappear before receive → treat missing or failed receives as benchmark errors.
- Remote cleanup may fail after successful receive → treat cleanup failure as a benchmark error so temporary device state is not silently left behind.
- Deferred HDC/device tests leave early integration risk → mitigate with strict local tests and later fake-HDC/device smoke tests once APIs settle.
- CSV-only output is less user-friendly than plots or Markdown → acceptable for v1 because CSV is stable, testable, and sufficient for later report layers.
