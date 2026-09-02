## Context

Users already construct benchmark flows by hand or with the Python programmatic API. On a device, `uitest uiRecord record` captures real touch/swipe/fling/key events via MMI input callback into `/data/local/tmp/record.csv`. This recorder can be driven by an offline helper CLI to produce a replayable `flow.json` without manual coordinate guessing.

## Goals / Non-Goals

**Goals:**
- Provide a user-friendly root-level CLI at `record.py` that starts/stops `uitest uiRecord` on-device and produces a valid `flow.json`.
- Allow recording duration to be bounded by either a timeout in seconds or a manual `y` + Enter prompt.
- Parse `record.csv` JSON lines through dedicated Pydantic recorder-event models before converting them into canonical `AppFlow` / command models and serializing via the existing benchmark schemas.
- Map `uitest` raw screen coordinates to normalized integer percentage coordinates using the recorder `windowBounds` row.
- Preserve the order of recorded inputs and the order in which recorded app identities appear.
- Derive stable app labels from recorded bundle/ability identity.
- Produce a predictable output filename: `flow-YYYYMMDD-HHMMSS-MILLISECONDS.json`.
- Handle recording failures gracefully and report incomplete data.

**Non-Goals:**
- Does NOT instrument running benchmarks or modify runner behavior.
- Does NOT insert `wait`, `snapshot`, `screenshot`, or other non-input commands; users edit the generated flow afterward.
- Does NOT support multi-finger gestures beyond the first finger data. Multi-finger records are recorded as-is but the converter uses only the first finger.
- Does NOT convert typed text into `text` or `input_text`; experiment showed soft-keyboard typing is recorded as pointer taps/double-taps on keyboard coordinates, not semantic text.

## Decisions

### 1. Recording signal: timeout + stdin prompt

```
┌──────────────┐      ┌───────────────────┐      ┌──────────────┐
│ Start HDC    │─────▶│ uitest uiRecord   │─────▶│ Wait for     │
│ shell        │      │ record -W false   │      │ stop signal  │
└──────────────┘      └───────────────────┘      │              │
                                                  │  timeout OR  │
                                                  │  stdin y     │
                                                  └──────┬───────┘
                                                         │
                                                         ▼
                                                  ┌──────────────┐
                                                  │ Terminate    │
                                                  │ subprocess   │
                                                  └──────┬───────┘
                                                         │
                                                         ▼
                                                  ┌──────────────┐
                                                  │ hdc shell    │
                                                  │ cat record   │
                                                  └──────┬───────┘
                                                         │
                                                         ▼
                                                  ┌──────────────┐
                                                  │ Parse +      │
                                                  │ convert +    │
                                                  │ write        │
                                                  └──────────────┘
```

- Launch `uitest uiRecord record -W false -c true` as an HDC subprocess whose stdout lines are parsed internally and not shown to the user.
- Treat the stdout line `Started Recording Successfully...` as the readiness signal.
- Use an internal non-configurable startup timeout of 120 seconds while waiting for readiness.
- Start the user-provided recording timeout only after readiness is observed.
- A separate Python thread reads stdin waiting for `y` + Enter.
- A second mechanism checks elapsed wall time against the optional timeout.
- Whichever **fires first** triggers subprocess `SIGINT`.
- After termination, `uitest uiRecord read` (or equivalent `hdc shell cat /data/local/tmp/record.csv`) retrieves the recorded JSON.

**Alternatives considered:**
- *using `-c true` and parsing stdout directly during recording.* The recorder output is human-readable, not machine-friendly, and `record.csv` provides unambiguous JSON. Parsing from the file is simpler and reliable.
- *terminating the local HDC subprocess without SIGINT.* An experiment confirmed `proc.send_signal(SIGINT)` stops `uitest uiRecord` after readiness; the local HDC process returned code `1` with no stderr, which should be accepted as a normal recorder stop when SIGINT was requested.
- *`uitest uiRecord record` with `point` mode.* `point` mode records raw coordinate streams, not gesture-level events. Full recording gives us already-classified `OP_TYPE` (click/fling/swipe) which maps cleanly to our schema.

### 2. Recorder event parsing and conversion mapping

Each non-empty `record.csv` JSON line is first parsed as a Pydantic `UIEvent` model with `EVENT_TYPE` constrained to `"key"` or `"pointer"`. Pointer records contain `finger_list` entries parsed as `FingerEvent` models. The observed recorder typo `FILEPAHT` is accepted alongside `FILEPATH`.

| recorder OP_TYPE     | flow command      | Notes                          |
|----------------------|-------------------|---------------------------------|
| `click`              | `tap`             | use `X_POSI`, `Y_POSI`        |
| `longClick`          | `long_tap`        | use `X_POSI`, `Y_POSI`        |
| `doubleClick`        | `double_tap`      | use `X_POSI`, `Y_POSI`        |
| `swipe`              | `swipe`           | start→end, VELO, LENGTH       |
| `drag`               | `drag`            | start→end, VELO               |
| `fling`              | `fling`           | start→end, VELO, LENGTH       |
| key event            | `key`             | KeyCode → Home/Back/Power map (see below) |
| `home`               | `key` Home        | recorded as gesture, not key event |
| `back`               | `key` Back       | same                            |
| `pinch`, `recent`    | SKIP (warn)       | no equivalent command          |
| other                | SKIP (warn)       | otherwise unsupported          |

**Key mapping**: The recorder stores keycodes as integers. Pragmatic mapping to string:

| Recorder KeyCode | flow key string |
|-------------------|-----------------|
| 2057              | Back            |
| 2066              | Home            |
| 2074              | Power           |

Other keycodes are skipped with a warning.

**Velocity and step length**: Recorder values are strings and may contain floats. Convert velocity with `round(float(VELO))`, clamp to `200..40000`, and emit strict `int`. Convert step length with `max(1, round(float(LENGTH)))` for fling payloads.

**Coordinates and identity**: Recorder stdout prints `windowBounds : (left,top,right,bottom)` at startup. Some recorder builds may also include a `windowBounds` CSV row, but observed device output omitted it from `record.csv`. The converter MUST accept bounds captured from recorder stdout and MAY also accept a bounds CSV row if present. If a record row has empty `BUNDLE` or `ABILITY`, the converter preserves the empty value because startup foreground ability does not necessarily correspond to the event. It then normalizes raw pixel coordinates into integer percentages required by the benchmark schema:

```text
x_pct = round((x - left) * 100 / (right - left))
y_pct = round((y - top) * 100 / (bottom - top))
```

Values are clamped to `0..100` after rounding. If bounds are missing or invalid, conversion fails because emitting raw pixels would produce an invalid or non-portable flow.

### 3. Generated flow structure

```json
{
  "$desc": "Recorded from device UI inputs. Add waits/snapshots/screenshots manually before benchmarking.",
  "flow": [
    {
      "label": "com_example_app_EntryAbility-1",
      "bundle": "com.example.app",
      "ability": "EntryAbility",
      "terminate": false,
      "commands": [
        ...
      ]
    }
  ]
}
```

- Preserve input order by app identity segments: consecutive supported records with the same recorded `BUNDLE` + `ABILITY` stay in the same `AppFlow`; when identity changes, start a new `AppFlow` in that encounter order.
- Every generated `AppFlow` uses `terminate: false` because the recorder preserves UI-input order rather than lifecycle intent.
- For each app identity segment, generate a schema-safe label from `f"{bundle}.{ability}-{n}"` by replacing non-label characters with `_`, where `n` is the number of times that same bundle/ability pair has appeared so far in the generated flow.
- If a supported record has empty `BUNDLE` or empty `ABILITY`, emit `""` for the missing field in the JSON output, emit a warning, and continue. Label generation uses the empty string as-is, so for an empty bundle the label becomes `_entryability-1`.
- `$desc` notes that the generated file contains recorded UI inputs only.
- The user is expected to edit the file after generation to add `wait`, `snapshot`, `screenshot`, and other benchmark-specific commands.

## Risks / Trade-offs

- **Timing is intentionally omitted.** Recorded human pauses are not converted to `wait` commands. → Document that this helper records only UI inputs and users add waits/snapshots/screenshots manually.
- **Coordinate normalization loses precision.** Integer percentages are portable but cannot represent every pixel exactly. → Round and clamp deterministically, matching existing flow schema expectations.
- **Recorder requires system capabilities.** `uitest uiRecord` uses MMI `AddMonitor` which needs system-level access. Standard user builds may not support recording. → Document that recordings work on developer/userdebug builds.
- **Only first finger tracked.** Swipe-type events with multi-finger data store only the first finger's start/end. → Document; multi-finger support could be added later.
- **Text input replays as keyboard taps.** Experiment showed typing `hello` produced pointer click/double-click records, not semantic text records. → Recorder preserves those UI inputs as taps/double-taps; users can manually replace them with `text` or `input_text` if desired.
- **Empty app identity can appear.** Experiment showed at least one supported event may have empty `BUNDLE`/`ABILITY`, likely because some gesture types do not call `GetFrontAbility()` per event. → Emit empty strings with warning instead of failing or substituting startup foreground ability, preserving the recorded event metadata while keeping the JSON schema-valid.
- **`uitest uiRecord` overwrites previous record.csv on each call.** Concurrent recording sessions are not supported. → Single process at a time; start-up check via `pidof uitest` could be added but is out of scope for MVP.

## Open Questions

None.
