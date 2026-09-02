## Context

The benchmark runner currently supports optional reboot preparation, screen-timeout prevention, and log capture. User feedback indicates reboot should be the normal/default path because benchmark results are more stable from a fresh device state. Devices can also remain asleep or on a lock/overlay screen after boot completion, so the reboot path needs an explicit wake routine before benchmark apps launch.

The current implementation direction is:

```text
if reboot:
  hdc target boot
  hdc wait
  wait bootevent.boot.completed
  power-shell wakeup
  directional fling up
  Back
always:
  power-shell timeout -o 60000000
if logs:
  configure hilog
```

## Goals / Non-Goals

**Goals:**
- Make reboot enabled by default while preserving `--no-reboot` as the opt-out.
- Add a production `Device.wakeup()` operation using `power-shell wakeup`.
- Specify and test the post-reboot wake routine: wakeup, upward directional fling, then Back.
- Keep wake/unlock gestures scoped to reboot preparation so non-reboot runs remain minimally invasive.
- Update docs/specs/tests to match the new default and wake behavior.

**Non-Goals:**
- Guarantee unlocking all possible secure lock screens.
- Add configurable unlock gestures or per-device unlock profiles.
- Change log collection defaults or output layout.
- Restore `--out`.

## Decisions

### Reboot defaults to enabled

The CLI default for `--reboot` becomes `True`; `--no-reboot` remains available for faster iteration or cases where reboot is undesirable. This matches user expectations that repeatable benchmark runs start from a clean device state.

Alternative considered: keep reboot disabled by default and document that users should pass `--reboot`. This leaves the stable path opt-in and is easy to forget.

### Wake routine is part of reboot preparation only

The wake routine runs after boot completion and before screen bounds are read:

```text
wakeup()
directional_fling("up", 20000, 20)
send_key("Back")
```

It is not run when reboot is disabled. This keeps regular no-reboot runs from unexpectedly changing foreground UI state beyond the app launches requested by the benchmark.

Alternative considered: always wake before every benchmark. That may help asleep devices but also makes no-reboot runs more invasive and less predictable for debugging.

### Wakeup is a device-layer operation

`Device.wakeup()` wraps `hdc shell power-shell wakeup` and raises on failure like other environment setup operations. The fling and Back steps reuse existing device operations rather than adding specialized unlock APIs.

Alternative considered: represent the entire sequence as one `Device.prepare_after_reboot()` method. Keeping wakeup as a primitive plus existing UI/key operations preserves the current small device API style and keeps tests explicit.

### Tests assert behavior, not full production HDC logs in runner tests

Device command translation tests should cover `Device.wakeup()` with fake HDC logging. Runner tests should use `FakeDevice` state/call recording to assert that reboot preparation invokes wakeup and follow-up gestures.

## Risks / Trade-offs

- **Risk:** The fixed fling/back sequence may not clear every device's boot-time UI.  
  **Mitigation:** Document it as a best-effort wake routine and keep future configurable unlock routines out of scope.

- **Risk:** Default reboot makes benchmark startup slower.  
  **Mitigation:** Keep `--no-reboot` as an explicit opt-out.

- **Risk:** Wakeup or gesture failure can fail the benchmark before AppFlows.  
  **Mitigation:** Fail early with the device-layer error so unstable preparation is visible rather than producing misleading results.
