## Context

Run-wide hilog streaming was introduced to replace per-AppFlow `hilog -r` / `hilog -x` dumps with one continuous stream to a device-local file. The current implementation starts a Python `multiprocessing.Process` that calls blocking `Device.hilog(remote_path)`. The child calls `os.setsid()`, and finalization attempts `os.killpg()` to stop both the child and its `hdc shell hilog` subprocess descendants.

That lifecycle is Unix-specific. `os.setsid()` and process-group signaling are unavailable on Windows, so the framework is not platform-independent. The pre-run-wide implementation avoided this because `hilog -x` was non-blocking, but reverting to that would lose continuous runtime log coverage and reintroduce hilog buffer-loss risk.

## Goals / Non-Goals

**Goals:**
- Preserve run-wide continuous hilog logging semantics.
- Stop using Unix-only host process APIs for hilog stream lifecycle.
- Manage the actual HDC subprocess directly with portable Python built-in APIs.
- Keep stream output device-local and avoid streaming hilog into host memory.
- Preserve stop-on-success, stop-on-failure, pending artifact receive, and error aggregation behavior.

**Non-Goals:**
- Reverting to per-AppFlow `hilog -r` and `hilog -x` dumps.
- Adding external process-management dependencies.
- Adding host-side log streaming or host-side log files.
- Solving every possible descendant process leak for arbitrary shell implementations; this change targets a portable best effort using the direct HDC process handle.

## Decisions

### Manage the HDC subprocess directly

Replace this shape:

```text
runner
  └─ multiprocessing.Process
       └─ HDC subprocess from hdc.shell("hilog > path")
```

with this shape:

```text
runner
  └─ child_processes: list[subprocess.Popen]
       └─ subprocess.Popen([hdc, "shell", "hilog > path"])
```

The runner should retain the generic Python process handle and stop it with shared process stop logic that terminates the direct HDC host subprocess, waits for bounded shutdown, then kills it if needed.

Rationale: Terminating the process that actually owns the blocking `hdc shell hilog` call is more direct and avoids needing a separate Python child process, wrapper object, or Unix process groups.

### Put subprocess ownership in the HDC/device layer

The HDC layer already owns subprocess invocation details for normal commands. Add a streaming-style operation there, exposed through `Device`, so the runner can request a run-wide hilog stream without constructing HDC command lines itself.

Possible internal shape:

```python
class Hdc:
    def start_shell(self, command: str) -> subprocess.Popen[Any]: ...

class Device:
    def start_hilog(self, remote_path: pathlib.PurePosixPath) -> subprocess.Popen[Any]: ...
```

The key boundary is that runner stores generic Python process handles in `child_processes`; no HDC-specific wrapper class is introduced.

### Use portable subprocess APIs only

The stop sequence should use Python built-ins that exist on supported platforms:

```text
if process already exited:
  check return code and report failure if non-zero
else:
  process.terminate()
  wait(timeout)
  if still running:
    process.kill()
    wait()
  if non-zero return code represents expected termination:
    do not treat it as stream failure
```

The exact return-code policy needs care: stopping a long-running stream by termination normally yields a non-zero or platform-specific return code. That should not be reported as a benchmark failure when stop was requested intentionally. Early exit before finalization should remain a failure.

### Preserve device-local redirection

The HDC command remains equivalent to:

```text
hdc shell "hilog > <remote_path>"
```

This keeps hilog output on the device and avoids unbounded host stdout capture. The managed subprocess should redirect host stdout/stderr to pipes or `subprocess.DEVNULL` in a way that does not block. If stderr is captured for diagnostics, it must be drained safely or bounded.

### Track generic child processes

Execution context should keep the existing `child_processes` concept, but store generic `subprocess.Popen` handles rather than `multiprocessing.Process` instances. Finalization should stop every tracked child process before receiving pending artifacts.

## Risks / Trade-offs

- Directly terminating `hdc` may not always terminate the remote shell command on every HDC/platform combination → Mitigation: preserve remote artifact receive behavior, test with real devices where possible, and document that this is portable best-effort host cleanup.
- Intentional termination may surface platform-specific non-zero return codes → Mitigation: distinguish early unexpected exit from requested stop; do not fail solely because a deliberately terminated stream has a non-zero code.
- Captured subprocess pipes can deadlock if not drained → Mitigation: use `DEVNULL` for blocking stream subprocess output unless diagnostics require bounded capture.
- Moving lifecycle ownership from runner to HDC/device layer changes test seams → Mitigation: add fake/stub stream handles and real HDC command translation tests.

## Migration Plan

- Keep runner child-process storage, but store direct `subprocess.Popen` handles for streaming HDC commands.
- Replace `_run_hilog` and process-group helpers with portable generic process stop logic.
- Keep result layout, CLI flags, and log artifact names unchanged.
- Existing outputs remain compatible because `logs/hilog.log` semantics are preserved.

## Open Questions

- Should the managed HDC stream use `stdout=DEVNULL, stderr=DEVNULL`, or capture stderr for early-failure diagnostics with bounded communication?
- What timeout should stop use: keep current 5 seconds or introduce a named constant?
- Should early stream exit be checked immediately after startup, or only during finalization?
