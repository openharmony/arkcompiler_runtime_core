## Why

`run.py` hardcodes the output directory as a relative `memmem-out-YYYYMMDD_HHMMSS_microseconds` name, so users cannot control where results land. A user-supplied absolute out dir cannot be passed to `hdc file recv` today: the host runs the Windows `hdc.exe` from WSL, WSL translates the exe's working directory but never absolute Linux argument paths, so an absolute local path would silently resolve against the wrong Windows drive.

## What Changes

- Add an optional `--out-dir` argument to `run.py`; omitting it keeps the current timestamped default directory name.
- Absolutize CLI path arguments (`--flow`, `--memmem-log-file`, `--out-dir`) at argument-parse time and absolutize `HDC_PATH` inside `.env` loading, so no path resolution depends on the working directory at run time.
- Verify-and-create becomes the benchmark runner's existing `mkdir(exist_ok=False)`; an already existing out dir fails the run with a friendly CLI error message.
- The benchmark runner changes its working directory to the out dir immediately after initializing local output, runs the benchmark, and restores the original working directory after the run.
- Local paths handed to HDC file-transfer operations during artifact receive become relative to the run working directory, which WSL translates location-correctly.
- Remote output directory naming becomes a host-timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` name generated independently of the local out dir instead of being derived from the local path.

## Capabilities

### New Capabilities
- None

### Modified Capabilities
- `benchmark-flow`: CLI gains `--out-dir`; the runner gains working-directory lifecycle (chdir to out dir, restore afterward, relative local paths for file transfers), replacing the "output SHALL be written to a timestamped `memmem-out-...` directory" requirement.
- `result-evidence`: remote output directory naming is no longer derived from the local out dir; it SHALL be a host-timestamped, per-run-unique name generated independently.
- `programmatic-interface`: `lib.run()` resolves `out_dir` against the caller's current working directory before execution.

## Impact

- `run.py`: new `--out-dir` argument, parse-time absolutization, friendly existing-dir error.
- `src/runner.py`: chdir/restore in `_pre_flow`/outer finally, relative recv paths, remote dir naming move, deletion of `_remote_out_dir_name` and `_REMOTE_OUT_PART_RE`.
- `src/result.py` / `lib.py`: independent remote naming generation in `create_result_store`; `lib.run` absolutizes `out_dir`.
- `src/config.py`: absolutize `HDC_PATH`.
- Tests: `runner_test.py:55` remote-dir assertion rewrite (timestamp format), new cwd-lifecycle and relative-recv assertions, `run_cli_test.py` out-dir parse/error scenarios. `device_test.py` and `result_test.py` untouched.
- Docs: README `--out-dir` description.