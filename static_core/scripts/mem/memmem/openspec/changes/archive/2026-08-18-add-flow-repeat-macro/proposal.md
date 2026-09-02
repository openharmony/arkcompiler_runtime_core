## Why

Long JSON benchmark flows become hard to maintain when users need repeated UI actions followed by snapshots or screenshots. The Python API already supports repetition through normal code, but some users prefer JSON-only benchmark definitions and need a small, safe authoring convenience.

## What Changes

- Introduce an `UnprocessedFlow` schema alongside the canonical `Flow` schema. The unprocessed schema tolerates repeat macro items in command arrays and label strings that are illegal in the canonical schema, such as labels containing loop variable placeholders. All schema types SHALL live in `src/schema.py`; the canonical `Flow` and command schemas keep identical semantics, with command classes refactored onto an internal generic payload base.
- Add a public preprocessing API `lib.preprocess_flow(flow: UnprocessedFlow) -> Flow` as the only official way to transform an unprocessed flow into a canonical `Flow`. The `lib` wrapper mirrors `lib.run()` by revalidating its input through `UnprocessedFlow` before delegating to the internal transformation, and input acceptance is enforced statically: the signature takes `UnprocessedFlow` only. The CLI SHALL use the same API.
- Add public builder wrappers for the unprocessed form — `lib.unprocessed_flow`, `lib.unprocessed_app_flow`, `lib.unprocessed_snapshot`, `lib.unprocessed_screenshot`, and `lib.repeat` — and export the unprocessed type names `UnprocessedFlow`, `UnprocessedAppFlow`, `Macro`, `RepeatMacro`, `UnprocessedSnapshotCommand`, and `UnprocessedScreenshotCommand` from the public facade, while macro payload structs and the unprocessed command union remain internal.
- Expand repeat macros into concrete commands before validating the canonical `Flow` model and before any device actions.
- Support explicit loop variable naming via `iter_var` and string substitution using `{<iter_var>}` inside repeated commands, replacing tokens with 0-based iteration numbers in the range `[0, n_iter-1]`.
- Keep numeric payloads strict: substitution happens only in repeated commands whose payload is itself a string and does not coerce strings into numbers.
- Flatten the `text` command payload from a single-field struct to a non-empty string (breaking for existing text-command payloads); the `key` command payload stays a struct because substitution can never produce a valid named key.
- Reject unknown keys in all schema models, so malformed items carrying both `macro` and `action` keys fail validation instead of being silently parsed as commands.
- `n_iter` SHALL be a non-negative integer with no upper bound; `n_iter` of zero and an empty macro body expand to nothing without error.
- Persist expanded canonical validated flow JSON at output `flow.json`; the unprocessed macro source is not persisted as a separate artifact in this change.
- Keep `lib.run()` and the runtime runner API accepting only canonical `Flow` models.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: CLI-authored flow JSON may contain repeat macro items in command arrays, which are expanded by `lib.preprocess_flow` before canonical flow validation and execution.
- `result-evidence`: CLI and programmatic output `flow.json` remains canonical validated flow JSON and contains the expanded flow when the input used macros.
- `testing-support`: Tests cover unprocessed flow schema validation, preprocessing through `lib.preprocess_flow`, expansion, canonical output, validation failures, and exported type names without changing existing programmatic API expectations.

## Impact

- Affected code: `run.py`, `lib.py`, `src/schema.py` (command classes refactored onto an internal generic payload base, the `text` payload flattened to a string, plus the new unprocessed schema types), new `src/preprocess.py` (transformation API only), and tests around schema, preprocessing, CLI, and canonical output.
- Existing runtime schema and runner execution remain unchanged: `src/schema.py` continues to define canonical `Flow`, and `src/runner.py` executes only concrete action commands.
- No new external dependencies are expected.
- Breaking change: `text` command payloads flatten from `{ "text": "..." }` to a plain non-empty string, so existing flow files using the struct form must be updated. The `key` command payload intentionally stays a struct; this change is accepted because the struct text form has no macro substitution benefit and the text-command payload shape is unused in practice.
