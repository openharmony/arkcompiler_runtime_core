## 1. CLI and config

- [x] 1.1 Add `--out-dir` optional argument (type `pathlib.Path`) to `run.py`; keep the default timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` name when omitted
- [x] 1.2 Absolutize `--flow`, `--memmem-log-file`, and `--out-dir` (including the default) against the invocation cwd at argument-parse time in `run.py`
- [x] 1.3 Absolutize `HDC_PATH` in `src/config.py` `load_config` (`src/config.py` may be `run.py`'s `load_config` depending on current layout)
- [x] 1.4 Wrap existing out-dir creation failure in `run.py` main with a friendly `FileExistsError` message naming the output directory; verify it exits with code 1
- [x] 1.5 Update `run.py --help` text to describe `--out-dir`

## 2. Runner working-directory lifecycle

- [x] 2.1 Capture `original_cwd = pathlib.Path.cwd()` at `run_benchmark` entry
- [x] 2.2 In `_pre_flow`, `os.chdir(store.local_out_dir())` immediately after `_initialize_local_output` finishes
- [x] 2.3 In `run_benchmark`'s outer `finally`, after `_post_flow` completes, `os.chdir(original_cwd)`
- [x] 2.4 In `_receive_pending_artifact`, pass `os.path.relpath(local_path)` (relative to the run working directory) as the local path to `device.recv_file`; keep Python-side parent mkdir on the absolute path
- [x] 2.5 Delete `_remote_out_dir_name` and `_REMOTE_OUT_PART_RE` from `src/runner.py`

## 3. Remote output naming

- [x] 3.1 Generate the remote out dir name in `create_result_store` as a host-timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` name independent of the local out dir, and pass it as `remote_out_dir` to `ResultStore`
- [x] 3.2 Verify no remaining references to local-out-dir-derived remote naming in `src/`

## 4. lib.run out_dir resolution

- [x] 4.1 In `lib.run`, resolve `out_dir` against the current working directory at entry (`pathlib.Path(out_dir)` joined with `Path.cwd()`), keeping behavior for already-absolute paths unchanged

## 5. Tests

- [x] 5.1 Rewrite `test/runner_test.py:55-56` to assert the remote out dir matches the timestamped name format instead of the old path-derived name
- [x] 5.2 Add runner-level tests: working directory equals the out dir during the run, and the process cwd equals the cwd captured at benchmark entry after the run (including on failure)
- [x] 5.3 Add a test asserting `recv_file` receives a run-relative local path (via the FakeHdc/FakeDevice recording pattern used in `test/device_test.py`)
- [x] 5.4 Add `run_cli_test.py` cases: `--out-dir` parsing and default behavior, existing out dir rejected with the friendly error, cwd restored after main-level run (restore cwd in tearDown for any test touching `main`)
- [x] 5.5 Add `lib_test.py` case: relative `out_dir` passed to `lib.run` resolves against the caller's cwd
- [x] 5.6 Run `make tests_full` and fix any mypy/strict issues

## 6. Docs

- [x] 6.1 Update README `run.py` section with `--out-dir` description and WSL path note
- [x] 6.2 Sync delta specs to main specs and archive the change (openspec workflow)