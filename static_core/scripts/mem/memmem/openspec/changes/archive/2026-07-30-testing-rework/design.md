## Context

The framework tests currently use local fake implementations in multiple test modules. `commands_test.py` and `runner_test.py` each define a `FakeDevice`, while `device_test.py` defines a `FakeHdc`. The runner fake relies on class-level state and monkeypatching because `run_benchmark()` creates a `Device` internally from `Config`.

This change consolidates fake behavior into shared test helpers under `test/mock/` and introduces a direct device-injection seam for benchmark execution.

## Goals / Non-Goals

**Goals:**
- Provide one reusable `FakeHdc` that logs HDC command calls and returns configured `HdcResult` values.
- Provide one reusable `FakeDevice` that models device state for runner and command tests.
- Refactor `run_benchmark()` to accept a device-compatible object directly.
- Remove duplicated local fake classes and runner monkeypatching.
- Keep CLI behavior unchanged.

**Non-Goals:**
- Change real `Device`, `Hdc`, or CLI runtime behavior.
- Add external test dependencies.
- Build a fully accurate OpenHarmony device simulator.
- Use `FakeDevice` for HDC command translation assertions.

## Decisions

### Dependency injection moves to `run_benchmark()`

`run.py` will remain responsible for loading configuration and constructing production dependencies:

```text
load_config() -> Hdc -> Device -> run_benchmark(flow_path, out_dir, device)
```

`run_benchmark()` will accept `device: Device` instead of `config: Config`. This removes the need for tests to patch `src.runner.Device` or inspect class-level fake instances.

Alternative considered: keep the current runner signature and use a shared fake with an `instances` registry. This preserves the public function signature but keeps hidden global test state and monkeypatching.

### `FakeHdc` is the only command logger

`FakeHdc` will live in `test/mock/hdc.py`, record calls as `list[list[object]]`, and return configured `HdcResult` values. `run()` calls will record only the raw HDC arguments, such as `["file", "send", ...]`, while `shell()` calls will record the shell transport arguments, such as `["shell", "pidof", "com.example"]`. Exact response lookup will use string keys such as `shell pidof com.example`. Prefix response lookup will support dynamic command arguments such as layout dump paths generated with `time.time_ns()`.

Alternative considered: model HDC filesystem behavior inside `FakeHdc`. This would make HDC tests more stateful than necessary. The current need is command logging plus response control.

### `FakeDevice` models state but does not log UI calls

`FakeDevice` will live in `test/mock/device.py` and model:
- `processes: dict[str, int]` mapping bundle names to PIDs.
- `screen: ScreenBounds` returned by `screen_bounds()` unless `screen_error` is set.
- `dirs: set[PurePosixPath]` for remote directories.
- `files: dict[PurePosixPath, str]` for remote file content.
- `invalid_bundles: set[str]` for launch calls that do not create a process.
- failure flags for directory creation, smaps capture, receive, and cleanup.

It will not log method calls. Tests that need to assert HDC command translation should use real `Device` with `FakeHdc`.

Alternative considered: one logging fake device for all tests. This makes orchestration tests assert implementation details and duplicates the command-translation responsibility already covered by `Device` tests.

### PID allocation is simple and deterministic

When `launch_app(bundle, ability)` is called for a valid bundle not already in `processes`, the fake assigns `1000 + len(processes)`. Repeated launches of the same bundle keep the existing PID. Invalid bundles do not add entries, so later `resolve_pid(bundle)` fails.

Alternative considered: random PID allocation to catch tests that hardcode PID values. Deterministic allocation is simpler and avoids nondeterministic failures. Tests should still read allocated PIDs from fake state when the exact value matters.

### Fake filesystem distinguishes directories from files

`FakeDevice` will use `dirs` and `files` instead of one flat list. `capture_smaps(pid, remote_path)` will fail unless `remote_path.parent` already exists in `dirs`, and `recv_file(remote_path, local_path)` will fail unless `remote_path` exists in `files`.

This keeps snapshot tests honest about remote directory creation before smaps capture.

### Timestamps use host time

`FakeDevice.timestamp()` will return `str(time.time_ns())`. Tests that need stable timestamp assertions can validate shape or override the method in a small local subclass if necessary.

## Risks / Trade-offs

- **Risk:** Changing `run_benchmark()` is an internal breaking API change.  
  **Mitigation:** Update all repository callers in the same change; CLI behavior remains unchanged.

- **Risk:** Prefix response lookup in `FakeHdc` could match more broadly than intended.  
  **Mitigation:** Exact responses take precedence, and tests should use narrow prefixes.

- **Risk:** Removing `FakeDevice` call logging requires reshaping command tests.  
  **Mitigation:** Keep UI command translation assertions in `Device` tests through `FakeHdc.calls`; keep command tests focused on orchestration state.

- **Risk:** `FakeDevice` may grow into a broad simulator over time.  
  **Mitigation:** Keep it limited to current framework semantics and add only state needed by tests.

## Open Questions

- Should `FakeDevice` expose a named constructor such as `FakeDevice.default()` for readability, or are constructor defaults sufficient?
