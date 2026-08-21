## Why

Benchmark scenarios are currently hand-authored even when the desired scenario already exists as real user interaction on a connected device. A recorder utility would let users capture taps, swipes, flings, and keys from OpenHarmony `uitest uiRecord` and convert them into replayable `flow.json` commands.

## What Changes

- Add a root-level CLI utility at `record.py` that records UI input events from a connected OpenHarmony device.
- Use device-side `uitest uiRecord record` as the only recording source and read `/data/local/tmp/record.csv` after recording stops.
- Support an optional `--timeout` recording duration in seconds.
- Prompt the user when recording is ready and allow manual stop by typing `y` and pressing Enter.
- Stop recording when either the timeout elapses or the manual stop prompt is answered.
- Parse recorded pointer/key JSON records and convert supported events into canonical benchmark `Flow` JSON without inserting waits.
- Preserve recorded input order and app identity encounter order when emitting `AppFlow` entries.
- Write `flow-YYYYMMDD-HHMMSS-MILLISECONDS.json` and print the generated filename.
- Treat unsupported recorder events and empty recorder app identities as skipped/substituted with user-visible warnings, emit empty strings for missing bundle/ability fields, and treat malformed supported events as fatal rather than producing invalid flow JSON.
- Keep generated `AppFlow.terminate` values set to `false` because the recorder preserves UI-input order rather than lifecycle intent.

## Capabilities

### New Capabilities
- `ui-input-recording`: Captures device UI input recordings and converts them into benchmark flow JSON.

### Modified Capabilities
- `benchmark-flow`: The command schema remains unchanged, but generated flow files become an officially supported source of valid flow JSON.
- `testing-support`: Tests cover recorder parsing, conversion, timing, and output-file behavior.

## Impact

- Adds `record.py` as a user-facing helper CLI next to `run.py`.
- Updates the root README with a `Record (record.py)` section and file-referenced headings for `Run (run.py)`, `Record (record.py)`, and `Programmatic API (lib.py)`.
- Reuses existing `.env` `HDC_PATH` loading behavior from `run.py` where practical.
- Uses the existing `Hdc`, `Flow`, `AppFlow`, and command schema models for validation and serialization.
- Requires connected OpenHarmony device with `/bin/uitest` and `uitest uiRecord` support.
- Does not change benchmark execution, public `lib.run()`, runner behavior, or canonical command schema.
