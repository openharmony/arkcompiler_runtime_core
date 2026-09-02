## Context

`run.py` currently hardcodes a relative, timestamped output directory (`memmem-out-YYYYMMDD_HHMMSS_microseconds`) and passes it into `lib.run()`. The HDC binary is a Windows executable (`hdc.exe`) invoked from WSL; WSL translates the working directory of spawned Windows processes but never absolute Linux paths given as program arguments. The only HDC call that receives a host-side local path is `hdc file recv` via `Device.recv_file`. Remote (device-side) paths under `/data/local/tmp` are already host-agnostic.

## Goals / Non-Goals

**Goals:**
- User-controlled output location via `--out-dir` that works on WSL, native Windows, and native Linux without platform detection.
- Keep the current default timestamped behavior when `--out-dir` is omitted.
- Make the runner self-contained: no host-path resolution depends on a caller's working directory.

**Non-Goals:**
- Resuming or appending into an existing out dir (existing dirs remain an error).
- Platform detection or path translation (e.g., `wslpath`) anywhere.
- Changes to remote storage layout, evidence layout, or report formats.

## Decisions

### 1. CLI absolutizes all path arguments at parse time
`run.py` resolves `--flow`, `--memmem-log-file`, and `--out-dir` (including the timestamped default) against the invocation working directory immediately at argument parsing. `src/config.py` absolutizes `HDC_PATH` when reading `.env`. This makes all path resolution independent of the later working-directory change and removes the flow/log-file/out-dir asymmetry in one move.

### 2. Working-directory lifecycle lives in the benchmark runner
The reproducibility problem is solved by construction: WSL translates the exe's working directory location-correctly (drive letter under `/mnt/...`, `\\wsl.localhost` otherwise), so relative local paths given to `hdc.exe` always resolve into the out dir — including cross-mount out dirs (e.g., under `/mnt/d`), which manual `relpath` from an unrelated cwd cannot guarantee.

`run_benchmark` captures `Path.cwd()` at entry. `_pre_flow` runs `chdir(store.local_out_dir())` immediately after `_initialize_local_output` — the ordering is forced: `_initialize_local_output` performs `mkdir(exist_ok=False)` (parents=True), which is both the "does not exist" check and the creation, and `mkdir(".")` would always fail. Everything between initialization and the flow (device preparation, verification, remote init, hilog start) is device-side only and cwd-independent. The outer `finally` restores the original working directory after `_post_flow` completes, so the restore also runs when `_post_flow` itself raises.

Alternatives considered:
- **Store rebase to `"."` after chdir** — rejected: `PendingArtifact`s captured in `_pre_flow` (hilog, runner.py:118-121) would hold stale absolute `local_base`s and need rewriting; also mutates the injected `ResultStore` contract.
- **No chdir; relative path from invocation cwd** — rejected: cross-mount out dirs (e.g., `/mnt/d/out` from a `/home/...` cwd) produce `..`-containing relative paths that escape the WSL-translated root and resolve wrong.

### 3. HDC-facing local paths are relativized at receive time
`_receive_pending_artifact` computes `os.path.relpath(local_path)` against the current (post-chdir) working directory and passes only that relative path to `Device.recv_file`. Python-side directory creation keeps working on the absolute path. Because the run working directory is the out dir, the relative path never contains `..` and never escapes the WSL-translated root. Chosen over store rebasing (see decision 2) because it is a single point of change and leaves `PendingArtifact` and the injected `ResultStore` untouched.

### 4. Out dir reported as a relative path in `hdc file recv` — same treatment reserved for `send_file`
`Device.send_file` is currently unused by the runner; if ever wired, it needs the same receive-time relativization.

### 5. `lib.run` resolves `out_dir` against the caller's cwd
`Path.cwd().joinpath(out_dir)` at `lib.run` entry protects programmatic callers who pass relative paths; it is idempotent for already-absolute paths (`Path.cwd() / abs == abs`), so the CLI's parse-time absolutization and this step coexist.

### 6. Remote out dir named independently of the local out dir
`create_result_store` generates the remote directory name as a host-timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` name instead of deriving it from the local path. `_remote_out_dir_name` and `_REMOTE_OUT_PART_RE` in `src/runner.py` are deleted. Per-run uniqueness comes from the microsecond timestamp; concurrent runs were already mutually exclusive because both drive one device.

### 7. Existing out dir surfaces a friendly CLI error
The runner's `mkdir(exist_ok=False)` is the single existence check (no CLI-side duplicate). `run.py` catches `FileExistsError` and prints a dedicated message instead of the raw traceback line.

### 8. Cwd lifecycle is self-contained in the runner
`generate_reports`/`average_reports` run after `run_benchmark` and write via absolute store paths, so the restore does not affect them; repeats iterations each chdir and restore without drift. Record.py is unaffected. Tests need plain assertions (`Path.cwd()` restored after a run, recv argv relative via the FakeHdc recording pattern) — no cwd fixtures.

## Risks / Trade-offs

- [Restore ordering regressed by a future refactor (receiving artifacts after restore would feed absolute paths to hdc)] → Final review gate: restore lives in the outer `finally` after `_post_flow`; a code comment and a runner-level test pinning recv-relative behavior guard it.
- [Concurrent runs sharing a generated remote name (same-microsecond timestamp)] → Practically impossible; single-device tool. Timestamp format matches current naming conventions.
- [Restore moves cwd for the CLI process] → Cosmetically irrelevant: `run.py` exits right after; behavior is now deterministic and test-friendly.
- [`FileExistsError` surface might mask other mkdir failures] → Only leaf-existence raises it; parent errors (permissions) propagate as before.

## Open Questions

None.