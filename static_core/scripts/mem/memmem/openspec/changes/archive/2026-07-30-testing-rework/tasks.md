## 1. Dependency Injection

- [x] 1.1 Change `run_benchmark` to accept a device-compatible object instead of `Config`.
- [x] 1.2 Remove HDC and Device construction from `src/runner.py`.
- [x] 1.3 Update `run.py` to load config, construct `Hdc`, construct `Device`, and pass the device to `run_benchmark`.
- [x] 1.4 Update CLI tests for the new wiring while preserving user-facing CLI behavior.

## 2. Shared Fake HDC

- [x] 2.1 Create `test/mock/hdc.py` with reusable `FakeHdc`.
- [x] 2.2 Implement `FakeHdc.calls` as `list[list[object]]`, with `run()` recording raw HDC args and `shell()` recording shell transport args.
- [x] 2.3 Implement `FakeHdc.default_result` for unmatched calls.
- [x] 2.4 Implement exact string response lookup for configured command responses.
- [x] 2.5 Implement prefix string response lookup for commands with dynamic arguments.
- [x] 2.6 Refactor `device_test.py` to import and use shared `FakeHdc`.
- [x] 2.7 Remove the local `FakeHdc` and `BadLayoutHdc` definitions from `device_test.py`.

## 3. Shared Fake Device

- [x] 3.1 Add reusable `FakeDevice` to `test/mock/device.py`.
- [x] 3.2 Model process state as `processes: dict[str, int]` keyed by bundle.
- [x] 3.3 Implement valid launch behavior that allocates deterministic PIDs with `1000 + len(processes)`.
- [x] 3.4 Implement repeated launch behavior that reuses an existing PID for the same bundle.
- [x] 3.5 Implement configurable `invalid_bundles` so invalid launches do not create process state.
- [x] 3.6 Implement `resolve_pid` and `pid_exists` from fake process state.
- [x] 3.7 Implement configured screen bounds and screen-error behavior.
- [x] 3.8 Model remote directories as `set[PurePosixPath]` and remote files as `dict[PurePosixPath, str]`.
- [x] 3.9 Implement `make_dir`, `capture_smaps`, `recv_file`, and `remove_dir` using fake filesystem state.
- [x] 3.10 Make `capture_smaps` fail when the remote path parent directory does not exist.
- [x] 3.11 Implement `timestamp()` with host `time.time_ns()`.
- [x] 3.12 Implement UI and key methods as no-op device-compatible methods.

## 4. Test Refactor

- [x] 4.1 Refactor `commands_test.py` to import shared `FakeDevice` and remove its local fake class.
- [x] 4.2 Keep command snapshot tests focused on snapshot metadata and fake filesystem behavior.
- [x] 4.3 Move or preserve UI command translation assertions through real `Device` plus `FakeHdc.calls`.
- [x] 4.4 Refactor `runner_test.py` to pass a `FakeDevice` directly to `run_benchmark`.
- [x] 4.5 Remove `mock.patch("src.runner.Device", ...)`, class-level fake state, and fake instance registries from runner tests.
- [x] 4.6 Update runner tests to assert output files, fake process state, fake filesystem state, and receive results through direct fake references.

## 5. Verification

- [x] 5.1 Run `source ".venv/bin/activate" && make test`.
- [x] 5.2 Run `source ".venv/bin/activate" && make tests_full`.
- [x] 5.3 Run `openspec validate "testing-rework"`.
