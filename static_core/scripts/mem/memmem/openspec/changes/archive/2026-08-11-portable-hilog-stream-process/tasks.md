## 1. Portable Process Handling

- [x] 1.1 Use generic child process handles for blocking HDC subprocesses without wrapper classes.
- [x] 1.2 Implement process stop using `Popen.terminate()`, bounded `wait()`, fallback `kill()`, and final `wait()`.
- [x] 1.3 Distinguish unexpected early process exit from intentional termination during stop.
- [x] 1.4 Ensure stream subprocess stdout/stderr handling cannot deadlock or stream hilog into host memory.

## 2. HDC and Device Integration

- [x] 2.1 Add an HDC-level method for starting a blocking shell command as a generic child process.
- [x] 2.2 Add a device-level method for starting run-wide hilog streaming to a required remote path.
- [x] 2.3 Ensure the started command remains equivalent to `hdc shell "hilog > <remote_path>"` and does not use `hilog -x`.
- [x] 2.4 Preserve command failure reporting for unexpected stream startup or early-exit failures.

## 3. Runner Lifecycle Integration

- [x] 3.1 Keep `child_processes` storage and replace `multiprocessing.Process` hilog children with generic process handles.
- [x] 3.2 Replace `_run_hilog`, process-group termination helpers, and multiprocessing-specific `_stop_process` usage with generic process stop logic.
- [x] 3.3 Remove hilog lifecycle dependencies on `os.setsid`, `os.killpg`, Unix signals, and multiprocessing process groups.
- [x] 3.4 Preserve stream start order after remote output setup and before AppFlow launch.
- [x] 3.5 Preserve finalization behavior: stop child processes on success/failure, combine stop errors, then receive pending artifacts.

## 4. Test Updates

- [x] 4.1 Update fake device support to return fake generic child process handles.
- [x] 4.2 Update runner tests to assert stream stop behavior without multiprocessing shared memory or Unix signals.
- [x] 4.3 Add HDC/device tests for hilog stream command translation.
- [x] 4.4 Add tests for terminate/wait/kill fallback behavior of generic process stop logic.
- [x] 4.5 Add tests for unexpected early stream failure reporting.
- [x] 4.6 Remove or rewrite tests that depend on `os.setsid`, `os.killpg`, Unix signals, or process-group cleanup.

## 5. Verification

- [x] 5.1 Run the project test suite.
- [x] 5.2 Run lint/typecheck commands used by the project.
- [x] 5.3 Run `openspec validate "portable-hilog-stream-process" --type change`.
- [x] 5.4 Run `openspec validate --all`.
