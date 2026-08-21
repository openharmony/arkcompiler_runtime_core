## Context

The benchmark framework currently launches each `AppFlow`, records app metadata, executes commands, optionally captures logs, and leaves app processes running unless the flow itself navigates away. Memory `snapshot` commands capture smaps for all previously launched labels whose stored PID is still alive, so earlier AppFlows can intentionally or accidentally affect later evidence. The framework also lacks visual evidence for confirming UI state.

Real-device exploration on Gallery showed that OpenHarmony `aa` does not provide a generic `stop` command for UI abilities: `aa stop` is invalid, and `aa stop-service` fails for non-service abilities. The working bundle-level termination command is `aa force-stop <bundle>`, but its success must be inferred from stdout because failed package lookups can still return host/shell status `0`. Screenshot probing showed `uitest screenCap -p <path>` produces PNG files and fits the existing UITest dependency better than `snapshot_display`, which only accepts `.jpeg`.

## Goals / Non-Goals

**Goals:**
- Require every `AppFlow` to declare whether its launched bundle should be terminated after that AppFlow.
- Terminate requested AppFlows after command execution and after per-AppFlow log dumping.
- Fail benchmarks when requested termination fails.
- Add a `screenshot` command that captures current screen state as raw evidence.
- Store screenshots as PNG files under `screenshots/<screenshot_label>.png`.
- Write `screenshot_metadata.json` preserving screenshot command order, timestamps, and artifact relative parts.
- Receive pending screenshot artifacts with existing deferred artifact transfer machinery.

**Non-Goals:**
- No separate terminate command inside `commands`.
- No default value for `terminate`; existing flows must be updated explicitly.
- No CSV reporting from screenshots.
- No app label or PID association in screenshot metadata.
- No nested screenshot directories.
- No use of `snapshot_display` for screenshot capture.

## Decisions

### AppFlow termination is lifecycle policy

Termination belongs on `AppFlow` because each AppFlow represents one launch lifecycle. A separate command would imply termination can occur mid-flow, but that would conflict with the current model where commands execute after one app launch and before post-flow evidence handling.

### `terminate` is required

The schema will require `terminate: bool` on every AppFlow. This is a breaking change, but it avoids hidden lifecycle behavior and forces benchmark authors to decide whether each launched app should remain alive for later snapshots.

### Device API uses intent-oriented naming

Expose `Device.terminate_app(bundle)` instead of `force_stop_app`. The device API names framework intent while hiding the OpenHarmony mechanism. Internally it uses `aa force-stop <bundle>` because device testing showed `aa stop` is unavailable and `aa stop-service` does not apply to UI abilities.

The success policy must inspect output, not just return code. A successful termination requires no HDC-level error and a stdout success marker such as `force stop process successfully`. Output containing `error:` or missing the success marker is treated as failure.

### Termination runs after log dumping

Post-flow work should preserve diagnostic evidence before terminating the process. `_post_app_flow` should attempt log dumping when enabled, then attempt termination when requested. If the AppFlow already failed, post-flow failures are combined with the original failure using the existing error-combination pattern.

### Screenshots are command-level visual evidence

A `screenshot` command uses a label payload like `snapshot`, but captures one whole-screen artifact rather than per-app/per-PID artifacts. Screenshot labels are unique only among screenshot commands. The screenshot metadata uses `artifacts: list[RelativeParts]` for consistency with snapshot metadata, even though each screenshot command currently stages exactly one artifact.

### Screenshot capture uses UITest PNG output

Use `uitest screenCap -p <remote_path>`. It is already available alongside existing UITest UI input and layout commands, produces PNG output, and fails when the parent directory is missing. Screenshot files use `.png` extension.

### Screenshot artifacts are a new pending artifact group

The existing `_PendingArtifacts` model can receive screenshots as another artifact group. Local and remote output initialization will create `screenshots/`, metadata writing will include `screenshot_metadata.json`, and cleanup remains unchanged.

## Risks / Trade-offs

- `aa force-stop` is named forcefully and may bypass graceful app lifecycle behavior. → The device does not expose a working generic UI-app `stop`; use `Device.terminate_app()` naming and strict stdout validation to avoid masking command failures.
- Requiring `terminate` breaks existing flow files. → This is intentional to make lifecycle state explicit; update README and schema tests.
- Screenshot commands add output volume. → Screenshots are only captured when explicitly requested.
- Screenshot receive failures may occur after metadata has been written. → This matches existing snapshot/log artifact behavior and keeps failure evidence discoverable.
- Screenshot labels unique only among screenshots may allow a screenshot and snapshot with the same label. → Metadata and directories are separate, so this is acceptable and matches the requested policy.
