## 1. Schema

- [x] 1.1 Refactor command classes onto the internal generic `_CommandBase[PayloadT]` in `src/schema.py`, keeping `action` literals, payload constraints, and the `Command` discriminated union behavior identical.
- [x] 1.2 Add payload type aliases needed as generic arguments, including `WaitPayload` for the wait payload constraint.
- [x] 1.3 Add `UnprocessedSnapshotCommand` and `UnprocessedScreenshotCommand` as `_CommandBase[str]` siblings without label validators.
- [x] 1.4 Add `UnprocessedCommand` as an `action`-discriminated union of the unprocessed snapshot/screenshot commands and the canonical non-label commands.
- [x] 1.5 Add the internal `_MacroBase[PayloadT]` generic base and `RepeatMacro(_MacroBase[RepeatMacroPayload])` with `macro` equal to `Literal["repeat"]`.
- [x] 1.6 Add `RepeatMacroPayload` with strict `iter_var` matching `^[A-Za-z_][A-Za-z0-9_]*$`, strict non-negative `n_iter` with no upper bound, and an `UnprocessedCommand` list.
- [x] 1.7 Add the `Macro` discriminated union on the `macro` field.
- [x] 1.8 Add `UnprocessedAppFlow` with lenient `label` and `commands: list[Macro | Command]`.
- [x] 1.9 Add `UnprocessedFlow` mirroring canonical `Flow` (`$desc` alias handling) without label or uniqueness validation.

## 2. Preprocessing API

- [x] 2.1 Add `preprocess_flow(flow: UnprocessedFlow) -> Flow` in `src/preprocess.py` that expands macros in place, preserves command order and the `desc`/`$desc` metadata, and validates the result through the canonical `Flow` model before returning.
- [x] 2.2 Implement `{<iter_var>}` substitution for repeated commands whose `payload` is itself a string only (flat payloads: unprocessed snapshot and screenshot labels and the `text` payload), using 0-based iteration numbers in the range `[0, n_iter-1]`; struct payloads are not traversed, object keys are never modified, and strings are never coerced into numbers.
- [x] 2.3 Ensure `n_iter` of zero and empty macro bodies expand to nothing without error.
- [x] 2.4 Add a validating `lib.preprocess_flow` wrapper that revalidates its input through `UnprocessedFlow` before delegating to `src.preprocess.preprocess_flow`, export `UnprocessedFlow`, `UnprocessedAppFlow`, `Macro`, `RepeatMacro`, `UnprocessedSnapshotCommand`, and `UnprocessedScreenshotCommand` from `lib.py`, and extend `__all__`; keep `UnprocessedCommand`, `RepeatMacroPayload`, and payload aliases internal.
- [x] 2.5 Add builder wrappers `lib.unprocessed_flow`, `lib.unprocessed_app_flow`, `lib.unprocessed_snapshot`, `lib.unprocessed_screenshot`, and `lib.repeat(n_iter, iter_var, commands)` mirroring the canonical facade.

## 3. CLI Integration

- [x] 3.1 Update `run.py` to read `--flow` JSON into `UnprocessedFlow` and call `lib.preprocess_flow` before `lib.run`.
- [x] 3.2 Keep `lib.run()` and programmatic builders accepting only canonical `Flow` models.
- [x] 3.3 Preserve output behavior so result `flow.json` contains expanded canonical validated flow JSON.

## 4. Tests

- [x] 4.1 Add schema tests for the `_CommandBase` refactor preserving canonical command behavior (validators, constraints, discriminator).
- [x] 4.2 Add tests proving canonical snapshot/screenshot commands reject illegal labels while unprocessed variants accept placeholder labels, including a guard against swapping the generic arguments.
- [x] 4.3 Add tests for valid macro expansion count, ordering, and 0-based `iter_var` substitution in the range `[0, n_iter-1]`.
- [x] 4.4 Add tests proving expanded snapshot and screenshot labels are validated for regex and uniqueness.
- [x] 4.5 Add tests proving string placeholders in numeric fields fail canonical validation instead of being coerced.
- [x] 4.6 Add tests for `n_iter` of zero, empty macro bodies expanding to nothing, and unbounded `n_iter` acceptance.
- [x] 4.7 Add tests for invalid `iter_var`, invalid `n_iter`, missing macro payload fields, and nested macro rejection.
- [x] 4.8 Add tests proving top-level illegal labels, including `{<iter_var>}` tokens outside macros, are rejected at unprocessed validation.
- [x] 4.9 Add CLI and evidence tests proving output `flow.json` contains expanded canonical commands without macro entries.
- [x] 4.10 Add lib tests proving `lib.preprocess_flow` transforms unprocessed flows, `lib.run()` remains canonical-flow-only, exported type names are available (`UnprocessedFlow`, `UnprocessedAppFlow`, `Macro`, `RepeatMacro`, `UnprocessedSnapshotCommand`, `UnprocessedScreenshotCommand`) while `UnprocessedCommand`, `RepeatMacroPayload`, payload aliases, and internal `_CommandBase`/`_MacroBase` are not.
- [x] 4.11 Add tests proving preprocessing preserves `$desc` metadata through expansion into the canonical flow.
- [x] 4.12 Add tests for unprocessed builder wrappers constructing valid models, including `lib.repeat` payload enforcement, and raising validation failures on invalid input.
- [x] 4.13 Add tests proving unknown keys in flows, app flows, commands, and payloads are rejected, including an item carrying both `action` and `macro` keys.

## 5. Documentation and Verification

- [x] 5.1 Update README flow JSON documentation with the two-form split, repeat macro syntax, `iter_var`, unbounded `n_iter`, empty-macro semantics, and `lib.preprocess_flow` usage.
- [x] 5.2 Document string-only substitution, non-coercion, post-expansion label validation, and the unbounded `n_iter` amplification risk.
- [x] 5.3 Run the project test suite.
- [x] 5.4 Run lint/typecheck commands used by the project.
- [x] 5.5 Run `openspec validate "add-flow-repeat-macro" --type change` and `openspec validate --all`.

## 6. Text payload flattening and preprocessing refinements

- [x] 6.1 Replace `TextCommand`'s struct payload with the flat `TextPayload` alias (`Annotated[str, Field(min_length=1, strict=True)]`) in `src/schema.py`, delete the old `TextPayload` struct class, reuse `TextPayload` for `InputTextPayload.text`, and keep `NamedKeyPayload` and `KeyCommand` unchanged.
- [x] 6.2 Update `src/commands.py` `execute_text` to pass the payload string directly to the device layer.
- [x] 6.3 Simplify `lib.text` to construct `TextCommand(action="text", payload=text)` from a string argument; keep `lib.key` and the `NamedKeyPayload` struct unchanged.
- [x] 6.4 Change `lib.preprocess_flow` into a validating wrapper that revalidates its input through `UnprocessedFlow.model_validate(flow.model_dump())` before delegating to `src.preprocess.preprocess_flow`; remove the isinstance type guard from `src/preprocess.py` so input acceptance is enforced statically by the typed signature.
- [x] 6.5 Change `expand_repeat_macro` to iterate `range(repeat.n_iter)` and substitute the 0-based iteration number.
- [x] 6.6 Restrict substitution to flat string payloads: substitute when the payload is itself a string, otherwise pass the command through unchanged without traversing payload structs.
- [x] 6.7 Update schema, preprocessing, lib, and CLI tests for the flat text payload, 0-based labels, the removed isinstance guard, and the validating wrapper; add tests for text placeholder substitution in macro bodies and wrapper revalidation of mutated input.
- [x] 6.8 Update README repeat-macro and commands documentation for 0-based substitution and the flat text payload.
- [x] 6.9 Rerun the project test suite, lint/typecheck commands, and `openspec validate --all`.
- [x] 6.10 Convert macro body unprocessed snapshot and screenshot commands to their canonical `SnapshotCommand`/`ScreenshotCommand` siblings after substitution so expanded commands are canonical instances.
