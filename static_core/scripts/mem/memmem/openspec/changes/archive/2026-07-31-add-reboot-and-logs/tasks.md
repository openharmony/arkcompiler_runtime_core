## 1. Options and CLI

- [x] 1.1 Add `src/options.py` with frozen `BenchmarkOptions(flow_path, out_dir, reboot=False, logs=True)`.
- [x] 1.2 Change full-run orchestration to accept `BenchmarkOptions` and `Device`, and lower-level `run_benchmark` to accept `BenchmarkOptions`, `ResultStore`, and `Device` with no default options argument.
- [x] 1.3 Update `run.py` to parse `--reboot`, `--no-reboot`, `--logs`, and `--no-logs`.
- [x] 1.4 Update `run.py` to create a timestamped output directory, construct `BenchmarkOptions`, create `ResultStore`, call lower-level `run_benchmark`, and generate reports.
- [x] 1.5 Update CLI tests for help text and default option behavior.

## 2. HDC Timeout Support

- [x] 2.1 Add `TIMEOUT_ERR_CODE = 127` to the HDC module.
- [x] 2.2 Add `timeout: int = -1` to `Hdc.run`.
- [x] 2.3 Return `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")` when subprocess execution times out.
- [x] 2.4 Add `timeout: int = -1` to `Hdc.shell` and forward it to `Hdc.run`.
- [x] 2.5 Add HDC tests for no-timeout behavior, timeout forwarding, and timeout result.

## 3. Device Environment Operations

- [x] 3.1 Add `Device.reboot()` using `hdc target boot`.
- [x] 3.2 Add `Device.wait_available(timeout_seconds)` using `hdc wait` with timeout.
- [x] 3.3 Add `Device.wait_boot_completed(timeout_seconds, poll_interval_seconds=1)` polling `param get bootevent.boot.completed`.
- [x] 3.4 Add `Device.disable_screen_timeout()` using `power-shell timeout -o 60000000`.
- [x] 3.5 Add `Device.configure_hilog()` using `hilog -Q pidoff` and `hilog -p off`.
- [x] 3.6 Add `Device.clear_hilog()` using `hilog -r`.
- [x] 3.7 Add `Device.dump_hilog(remote_path)` using `hilog -x > <remote_path>`.
- [x] 3.8 Add the requested comment above `dump_hilog()` noting future `-P <pid>` app-only filtering.
- [x] 3.9 Add device tests for reboot, wait, boot-complete success/failure, screen timeout, hilog configure, hilog clear, and hilog dump.

## 4. Result Paths and Log Artifacts

- [x] 4.1 Add shared `RelativeParts = list[str]` to result models and store pending logs in `ExecutionContext.logs`.
- [x] 4.2 Add local log path helpers for `logs/<app_label>.log`.
- [x] 4.3 Add remote log path helpers mirroring `logs/<app_label>.log` under the remote run directory.
- [x] 4.4 Update result store tests for local and remote log paths.

## 5. Runner Environment and Logs

- [x] 5.1 Add environment preparation helper that optionally reboots, always disables screen timeout, and optionally configures hilog.
- [x] 5.2 Add per-AppFlow helper to clear hilog before execution when logs are enabled.
- [x] 5.3 Add remote output initialization and per-AppFlow helper to dump hilog and record pending log artifact after execution when logs are enabled.
- [x] 5.4 Ensure log dumping is attempted when an AppFlow fails and logs are enabled.
- [x] 5.5 Preserve original AppFlow failures when log dumping also fails, using a combined error when practical.
- [x] 5.6 Add shared best-effort pending artifact receive flow for snapshot and log artifacts before remote cleanup.
- [x] 5.7 Make log receive failure fail the benchmark when logs are enabled.
- [x] 5.8 Skip all hilog setup, clear, dump, tracking, and receive behavior when logs are disabled.
- [x] 5.9 Update runner tests for reboot enabled/disabled, screen timeout always running, logs enabled default, logs disabled, successful log receive, and log failure behavior.

## 6. Report Sorting

- [x] 6.1 Sort breakdown CSV rows by `Size_total_for_tag` descending and tag ascending for ties.
- [x] 6.2 Update report tests to assert size-descending breakdown order.

## 7. Test Fakes

- [x] 7.1 Update `FakeHdc.run` and `FakeHdc.shell` to accept timeout parameters.
- [x] 7.2 Add `timeout_after` support to `FakeHdc` returning `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")` when timeout is too small.
- [x] 7.3 Update `FakeDevice` with reboot, wait, boot-complete, screen-timeout, hilog configure, hilog clear, and hilog dump methods.
- [x] 7.4 Add `FakeDevice` state and failure knobs for new environment and hilog operations.
- [x] 7.5 Update existing tests to use `BenchmarkOptions`.

## 8. Documentation and Verification

- [x] 8.1 Update README CLI documentation for `--reboot`, `--no-reboot`, `--logs`, and `--no-logs`.
- [x] 8.2 Update README output layout documentation to include `logs/<app_label>.log`.
- [x] 8.2a Update active artifacts for timestamped output naming, injected `ResultStore`, report generation outside the runner, and remote/local output-name mirroring.
- [x] 8.3 Run `source ".venv/bin/activate" && make test`.
- [x] 8.4 Run `source ".venv/bin/activate" && make tests_full`.
- [x] 8.5 Run `openspec validate "add-reboot-and-logs"`.
