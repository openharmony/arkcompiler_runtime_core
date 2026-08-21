## 1. Smaps Analysis Foundation

- [x] 1.1 Create `src/__init__.py`, `src/smaps.py`, `test/smaps_test.py`, and multiple smaps fixtures under `test/fixtures/`.
- [x] 1.2 Implement immutable `MemProfile` and `SmapsSummary` dataclasses in `src/smaps.py`.
- [x] 1.3 Implement `parse_smaps_text(text: str) -> SmapsSummary` with total Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous aggregation.
- [x] 1.4 Implement per-tag smaps breakdown aggregation, including stable tagging for mappings without explicit path tags.
- [x] 1.5 Add smaps parser tests covering multiple mappings, repeated tags, different tags, zero values, missing supported fields, and unsupported fields.
- [x] 1.6 Run `make tests_full` and ensure it passes.

## 2. Project Skeleton

- [x] 2.1 Create importable modules `src/config.py`, `src/schema.py`, `src/hdc.py`, `src/device.py`, `src/commands.py`, `src/result.py`, `src/report.py`, and `src/runner.py`.
- [x] 2.2 Keep `run.py` thin and import-safe without benchmark business logic.
- [x] 2.3 Run `make tests_full` and ensure it passes.

## 3. Config and Flow Schema

- [x] 3.1 Implement `Config` and `load_config(env_path: pathlib.Path) -> Config` in `src/config.py`.
- [x] 3.2 Add an env-file parsing dependency to `requirements.txt` if a dedicated parser is used.
- [x] 3.3 Implement pydantic action-discriminated `Command`, `AppFlow`, and `Flow` models in `src/schema.py`.
- [x] 3.4 Implement schema validation that command payloads match action-specific types and app labels and snapshot labels match `^[A-Za-z0-9_-]+$`.
- [x] 3.5 Keep schema as pydantic models only; perform JSON loading in the runner with `Flow.model_validate`.
- [x] 3.6 Add `test/config_test.py` covering valid `.env`, missing `.env`, missing `HDC_PATH`, and empty `HDC_PATH`.
- [x] 3.7 Add `test/schema_test.py` covering valid flow, missing required fields, order preservation, invalid app labels, invalid snapshot labels, action-specific payload validation, and unknown actions via direct model validation.
- [x] 3.8 Run `make tests_full` and ensure it passes.

## 4. ResultStore

- [x] 4.1 Implement pydantic `ProcessMetadata`, `ProcessSnapshot`, and side-effect-free `ResultStore` in `src/result.py`.
- [x] 4.2 Move local output directory creation, snapshot directory creation, flow copy, and metadata writing to the runner.
- [x] 4.3 Implement explicitly prefixed local path APIs for flow, snapshots, process metadata, snapshot artifacts, and breakdown artifacts.
- [x] 4.4 Implement explicitly prefixed remote path APIs for remote output, process snapshot directories, snapshot artifacts, and pending artifact receive paths.
- [x] 4.5 Implement local snapshot layout parsers for tracked PIDs and per-process snapshots.
- [x] 4.6 Add `test/result_test.py` covering local paths, remote paths, local parsing, and duplicate labels.
- [x] 4.7 Run `make tests_full` and ensure it passes.

## 5. CSV Reports

- [x] 5.1 Implement `SummaryRow` and internal snapshot report aggregation in `src/report.py`.
- [x] 5.2 Implement report collection through `ResultStore` local parsing APIs and pydantic `ProcessMetadata` parsing.
- [x] 5.3 Implement `write_summary_csv` with one total row per collected snapshot.
- [x] 5.4 Implement `write_breakdown_csv` with header `tag,Size_total_for_tag,Rss_total_for_tag,Pss_total_for_tag,Referenced_total_for_tag,Shared_total_for_tag,Private_total_for_tag,Swap_total_for_tag,Anonymous_total_for_tag`.
- [x] 5.5 Implement `generate_reports(store: ResultStore) -> None` to write `summary.csv` and all breakdown CSVs.
- [x] 5.6 Add `test/report_test.py` covering summary CSV rows, per-snapshot breakdown CSVs, and duplicate app labels.
- [x] 5.7 Run `make tests_full` and ensure it passes.

## 6. HDC Wrapper

- [x] 6.1 Implement immutable `HdcResult` with `returncode`, `stdout`, and `stderr` fields.
- [x] 6.2 Implement `Hdc.run(*args: str) -> HdcResult` to execute `<HDC_PATH> <args...>`.
- [x] 6.3 Implement `Hdc.shell(*args: str) -> HdcResult` to execute `<HDC_PATH> shell <args...>`.
- [x] 6.4 Do not add HDC mocking tests in this phase.
- [x] 6.5 Run `make tests_full` and ensure it passes.

## 7. Device Operations

- [x] 7.1 Implement `LaunchedProcess` and `Device` in `src/device.py`.
- [x] 7.2 Implement `launch_app(bundle, ability)` using HDC/OpenHarmony launch command patterns from legacy `~/memmem`.
- [x] 7.3 Implement `resolve_pid(bundle, excluded_pids) -> int` using PID lookup patterns from legacy `~/memmem`, excluding PIDs already tracked by the framework.
- [x] 7.4 Implement `pid_exists(pid) -> bool` by checking `/proc/<pid>` on device.
- [x] 7.5 Implement device timestamp retrieval for execution and snapshot timestamps.
- [x] 7.6 Implement remote directory creation and removal for `ResultStore.remote_out_dir()` under `/data/local/tmp/memmem-<run_id>/`.
- [x] 7.7 Implement `capture_smaps(pid, remote_path: pathlib.PurePosixPath) -> bool` through device-local `cat /proc/<pid>/smaps > <remote_path>`.
- [x] 7.8 Implement `send_key(payload: dict[str, str]) -> None` using schema-validated command payloads and HDC key event behavior.
- [x] 7.9 Implement `send_file(local_path, remote_path: pathlib.PurePosixPath)` and `recv_file(remote_path: pathlib.PurePosixPath, local_path)` using `hdc file send` and `hdc file recv`.
- [x] 7.10 Do not add device mocking tests in this phase.
- [x] 7.11 Run `make tests_full` and ensure it passes.

## 8. Command Execution

- [x] 8.1 Implement `ExecutionContext` in `src/commands.py` with a `snapshots` list of `SnapshotRelativeParts`.
- [x] 8.2 Implement `execute_command` dispatch for `wait`, `snapshot`, and `key` actions.
- [x] 8.3 Implement `wait` sleep behavior using schema-validated payloads.
- [x] 8.4 Implement `snapshot` tracked PID iteration, missing PID skip behavior, device snapshot timestamp retrieval, remote artifact path generation, device-local smaps capture, and context snapshot recording using schema-validated payloads.
- [x] 8.5 Implement `key` delegation to `Device.send_key` using schema-validated payloads.
- [x] 8.6 Implement schema-level failure for unknown command actions.
- [x] 8.7 Do not add command mocking tests in this phase.
- [x] 8.8 Run `make tests_full` and ensure it passes.

## 9. Runner

- [x] 9.1 Implement `run_benchmark(flow_path: pathlib.Path, out_dir: pathlib.Path, config: Config) -> None` in `src/runner.py`.
- [x] 9.2 Wire flow loading, result store initialization, HDC construction, device construction, remote execution timestamp retrieval, remote run directory creation, tracked process state, and pending artifact state.
- [x] 9.3 Launch each `AppFlow`, resolve PID immediately while excluding already tracked PIDs, write process metadata, and execute commands in order.
- [x] 9.4 Fail the benchmark when PID cannot be resolved immediately after launch.
- [x] 9.5 After all flows finish, receive every pending snapshot artifact into the output directory and fail if any receive fails.
- [x] 9.6 Remove the remote run directory after successful artifact receive and fail if cleanup fails.
- [x] 9.7 Generate reports after artifact receive and cleanup succeed.
- [x] 9.8 Run `make tests_full` and ensure it passes.

## 10. CLI

- [x] 10.1 Implement `run.py` argument parsing for required `--flow` and optional `--out`.
- [x] 10.2 Implement timestamped default output directory when `--out` is omitted.
- [x] 10.3 Load `.env` configuration and call `run_benchmark` from `run.py`.
- [x] 10.4 Add `test/run_cli_test.py` covering negative CLI cases without Device/HDC mocking.
- [x] 10.5 Run `make tests_full` and ensure it passes.

## 11. Final Validation

- [x] 11.1 Run `make tests_full` and ensure it passes.
- [x] 11.2 Run `python run.py --help` and verify the CLI help is useful.
- [x] 11.3 Run `python run.py --flow missing.json` and verify the error is clear.
- [x] 11.4 If an HDC target is available, run a minimal device smoke test with a simple flow.
