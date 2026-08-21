## 1. Device Hilog API

- [x] 1.1 Add `Device.hilog(path: pathlib.PurePosixPath) -> None` using blocking command `hilog > <path>` through HDC shell.
- [x] 1.2 Remove production use of `Device.clear_hilog()` and `Device.dump_hilog()`.
- [x] 1.3 Add device command translation tests for `Device.hilog(remote_path)`.
- [x] 1.4 Remove or update device tests for per-AppFlow hilog clear/dump behavior.

## 2. Result Paths and Pending Artifact Model

- [x] 2.1 Add `ResultStore.local_hilog_path() -> pathlib.Path` for `logs/hilog.log`.
- [x] 2.2 Add `ResultStore.remote_hilog_path() -> pathlib.PurePosixPath` for `<remote_out_dir>/logs/hilog.log`.
- [x] 2.3 Add `ResultStore.hilog_relative_parts() -> RelativeParts` returning `["hilog.log"]`.
- [x] 2.4 Remove or leave unused per-AppFlow log path helpers after updating all call sites.
- [x] 2.5 Add result path tests for the fixed hilog paths.

## 3. Runner Lifecycle

- [x] 3.1 Keep remote output initialization before stream startup so remote `logs/` exists before streaming.
- [x] 3.2 Start a `multiprocessing.Process` that calls `Device.hilog(store.remote_hilog_path())` when logs are enabled.
- [x] 3.3 Record exactly one pending log artifact with `store.hilog_relative_parts()` when logs are enabled.
- [x] 3.4 Stop the hilog process during finalization on successful benchmarks.
- [x] 3.5 Stop the hilog process during finalization when AppFlow execution, metadata writing, artifact receive, or cleanup fails after stream startup.
- [x] 3.6 Stop the hilog process group with `SIGTERM -> join(timeout=5) -> SIGKILL -> join()`, fall back to process `terminate()` / `kill()` when needed, and aggregate stop errors with existing combined-error behavior.
- [x] 3.7 Remove per-AppFlow hilog clear before AppFlow execution.
- [x] 3.8 Remove per-AppFlow hilog dump after AppFlow execution.
- [x] 3.9 Keep pending log receive in `_receive_all_pending_artifacts()` so `logs/hilog.log` is received with other pending artifacts.
- [x] 3.10 Update runner tests for successful streaming, disabled logs, startup health failure before stream start, AppFlow failure after stream start, receive failure, cleanup failure, and AppFlow termination order.
- [x] 3.11 Refactor runner lifecycle into `_pre_flow`, `_flow`, and `_post_flow` with child processes and flat pending artifacts owned by `ExecutionContext`.
- [x] 3.12 Replace snapshot/screenshot-specific execution context metadata lists with command-level `ExecutionContext.artifact_metadata` and shared `ArtifactMetadataFile` emission under artifact directories.
- [x] 3.13 Replace snapshot/screenshot-specific metadata path helpers with `ResultStore.local_artifact_metadata_path(path)`.

## 4. Test Support

- [x] 4.1 Add fake `Device.hilog(path)` support that stores fake log content at the remote path.
- [x] 4.2 Add fake hilog failure knob for blocking hilog streaming.
- [x] 4.3 Add real-multiprocessing tests for process start, terminate/join timeout handling, kill fallback, and artifact preservation without patching the runner process factory.
- [x] 4.4 Remove fake-device clear/dump hilog state that is no longer used.

## 5. Documentation and Verification

- [x] 5.1 Update CLI defaults so `--no-reboot` is the default option value while log collection remains enabled by default and explicit boolean controls are preserved.
- [x] 5.2 Update README CLI/output layout text from per-AppFlow logs to run-wide `logs/hilog.log` and document default `--no-reboot` / `--logs` behavior.
- [x] 5.3 Update OpenSpec main specs after implementation.
- [x] 5.4 Run `source ".venv/bin/activate" && make test`.
- [x] 5.5 Run `source ".venv/bin/activate" && make tests_full`.
- [x] 5.6 Run `openspec validate "stream-run-hilog"`.
