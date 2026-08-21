## Context

`run.py` currently reads JSON and validates it directly into the canonical `Flow` model. `Flow`, `AppFlow`, and `Command` in `src/schema.py` represent runtime/canonical benchmark data, and `src/runner.py` executes only concrete commands. Output initialization persists the canonical validated model to `flow.json`.

Users can already express loops through the public Python API, but JSON-only users must manually duplicate repeated command blocks. The desired feature is an authoring convenience, not a new device action. Because macro bodies substitute loop variables into label strings, the authored form must tolerate labels that are illegal in the canonical schema, such as `"shot_{i}"`.

## Goals / Non-Goals

**Goals:**
- Let flow JSON contain repeat macros inside `UnprocessedFlow` command arrays.
- Use a separate `UnprocessedFlow` schema for the authored form so the canonical `Flow` and `Command` schema semantics stay unchanged.
- Provide `lib.preprocess_flow(flow: UnprocessedFlow) -> Flow` as the only official transformation from the unprocessed form to the canonical form; the CLI goes through the same API.
- Expand macros before canonical `Flow` validation and before any device action.
- Keep existing runtime command execution unchanged.
- Keep output `flow.json` as expanded canonical validated flow JSON.

**Non-Goals:**
- Supporting numeric expressions or arithmetic.
- Coercing substituted string values into numbers.
- Supporting nested macros.
- Sanitizing labels: illegal characters that survive expansion fail canonical validation.
- Adding a general JSON template language or external template dependency.
- Persisting the unprocessed macro source as a separate result artifact.

## Decisions

### Use a separate UnprocessedFlow schema alongside the canonical schema

All schema types live in `src/schema.py` because it is the home of user JSON schema. The authored form is:

```text
UnprocessedFlow
UnprocessedAppFlow
UnprocessedCommand           (macro body items)
Macro                        (discriminated union of macro entries)
RepeatMacro
RepeatMacroPayload
UnprocessedSnapshotCommand
UnprocessedScreenshotCommand
```

The unprocessed schema validates the authored shape, including macro envelope fields, `iter_var`, `n_iter`, and macro body commands. Expansion then constructs a normal canonical `Flow`, validated by the existing runtime schema.

Alternative considered: expand raw dict/list JSON directly before validation. Rejected because it would duplicate type checks manually and reduce code safety.

### Command classes share an internal generic payload base

All canonical command classes are refactored onto `_CommandBase[PayloadT]`, a private pydantic generic class holding only `payload`. Concrete subclasses declare their `action` literal and keep existing payload constraints as the generic type argument:

```text
_CommandBase[PayloadT]
WaitCommand(_CommandBase[WaitPayload])
SnapshotCommand(_CommandBase[Label])          # label via StringConstraints pattern
KeyCommand(_CommandBase[NamedKeyPayload])
TextCommand(_CommandBase[TextPayload])            # TextPayload = Annotated[str, Field(min_length=1, strict=True)]
InputTextCommand(_CommandBase[InputTextPayload])
...
```

Label and loop-variable validation use the same construction: `Label` and `IterVar` are both `Annotated[str, StringConstraints(pattern=..., strict=True)]` aliases, so snapshot/screenshot/app labels and `iter_var` are validated identically instead of some via field validators and some via constraints. This is an inheritance-only refactor: canonical `Command`, `AppFlow`, and `Flow` keep identical semantics, `action` literals, and the `action` discriminator. The unprocessed variants are sibling subclasses with the same generic base minus label validation:

```text
UnprocessedSnapshotCommand(_CommandBase[str])     # no Label constraint
UnprocessedScreenshotCommand(_CommandBase[str])   # no Label constraint
```

Canonical and unprocessed siblings are distinct runtime classes, so `isinstance`-based validation in canonical `Flow` keeps working. The `Label`-versus-`str` generic argument is the only difference between the two snapshot forms; a schema test guards against swapping them.

Alternative considered: parameterize every schema class over its label type (full generics). Rejected: `isinstance` narrowing degrades to `Any`, canonical and unprocessed forms share runtime class identity, and strict-mode mypy friction increases. Alternative considered: duplicate the two label-bearing command classes. Rejected because the generic base removes the duplication while keeping concrete classes.

### Flatten the text command payload only

The `text` command payload flattens from a single-field struct to a plain string via the `TextPayload` alias (`Annotated[str, Field(min_length=1, strict=True)]`); the old `TextPayload` struct class is removed and `InputTextPayload.text` reuses the same alias. `lib.text` then constructs `TextCommand(action="text", payload=text)` with a string argument. This is a breaking payload shape change accepted because the struct form has no macro benefit — substitution targets payload string values, and a one-field struct only adds indirection — and the text-command payload shape is unused in practice.

The `key` command payload intentionally stays the `NamedKeyPayload` struct. Flattening `key` would provide no macro benefit: `NamedKeyPayload.key` is a `Literal["Home", "Back", "Power"]`, so any `{<iter_var>}` token in a key payload fails macro body validation before expansion and can never be substituted into a valid key. A breaking change with no substitutable surface would only churn existing flows, recorder output, and the base spec requirement. `InputTextCommand` also stays a struct because its payload is multi-field (`x_pct`, `y_pct`, `text`).

Alternative considered: flatten `key` to the same literal form as `text`. Rejected: substitution can never produce a valid named key from a token, so flattening yields no macro benefit while breaking existing `flow.json` files, recorder output, and the base `benchmark-flow` requirement.

### Macro envelope uses the same generic payload base pattern

`RepeatMacro` follows the same internal pattern as commands via `_MacroBase[PayloadT]` so future macro kinds do not require a refactor:

```text
_MacroBase[PayloadT]
RepeatMacro(_MacroBase[RepeatMacroPayload])   # macro: Literal["repeat"]
```

### Macro shape uses explicit loop variable naming

```json
{
  "macro": "repeat",
  "payload": {
    "iter_var": "i",
    "n_iter": 10,
    "commands": []
  }
}
```

`iter_var` MUST match `^[A-Za-z_][A-Za-z0-9_]*$`. `n_iter` is a strict non-negative integer with no upper bound; `n_iter` of zero and an empty `commands` list are valid and expand to nothing. The unbounded upper limit is an accepted trade-off: preprocessing is a pure function and the caller controls input size; the README documents the amplification risk.

### Top-level items are canonical commands or macros; macro bodies are unprocessed commands

`UnprocessedAppFlow.commands` items are `Macro | Command`: outside a macro there is no loop variable, so an illegal label there can never be substituted — rejecting it at unprocessed validation is an early, correct error. Macro body items are `UnprocessedCommand`: the lenient snapshot/screenshot variants allow placeholder labels, while all other commands are the canonical classes. Because the body type contains no macro member, nested macros are rejected structurally rather than by explicit validation.

### All schema models reject unknown keys

Every model in `src/schema.py` — canonical commands, macro and payload structs, `Flow`, `AppFlow`, and their unprocessed counterparts — sets `extra="forbid"` (the `_CommandBase` and `_MacroBase` bases carry it; the flow and app-flow models add it to their config). An item that carries both `action` and `macro` keys therefore fails every union member — each sees the other key as unknown — so the malformed item is rejected with a validation error instead of silently parsing as a command. Existing flows in the wild do not carry unknown keys (no `$schema` keys in the codebase or fixtures), so this changes no valid existing input.

### Substitute flat string payloads only

During expansion, `{<iter_var>}` is replaced with the 0-based iteration number in the canonical range `[0, n_iter-1]` only in repeated commands whose `payload` is itself a string — flat string payloads, i.e. unprocessed snapshot and screenshot labels and the `text` command payload: `n_iter` of `3` yields `shot_0`, `shot_1`, `shot_2`. Struct payloads are never traversed: string fields inside a payload struct such as `input_text.text` are not substituted. Command envelope fields (`action`, `macro`), macro payload fields, and object keys are never substituted. Integers, floats, booleans, and null values are not modified. A replace is a no-op when the token is absent.

This supports generated labels and repeated text input (via the flat `text` command payload) while preserving strict numeric schema validation. Because macro body commands reuse the canonical command classes with strict payload types, a string such as `"{i}"` in a numeric field such as `wait.payload` fails macro body validation before expansion — strings are never coerced into numbers. There is no escaping syntax for a literal `{<iter_var>}` token; this is accepted because labels cannot contain `{`/`}` anyway.

Alternative considered: coerce substituted strings into numbers. Rejected because it conflicts with the existing strict schema and creates surprising type behavior.
Alternative considered: recursive substitution into every string value inside payload structs. Rejected: only flat string payloads (labels and `text`) benefit from macro substitution, and struct traversal would silently rewrite strings inside payloads such as `input_text.text`, making macro behavior implicit; flat-only substitution keeps the substitution surface explicit.
Alternative considered: expression objects for numeric parameters. Deferred as a separate possible future change because it is substantially larger.

### lib.preprocess_flow is the only official transformation API

`src/preprocess.py` owns the transformation only: `preprocess_flow(flow: UnprocessedFlow) -> Flow` (the parameter cannot be named `in`, a Python keyword). Expansion mirrors the runtime command dispatcher shape: a single `expand_item` entrypoint dispatches with `isinstance` to one handler per macro kind (`expand_repeat_macro`) and passes canonical commands through unchanged; handlers return concrete command model instances that preserve their types through expansion instead of round-tripping the whole flow through dicts. Substitution applies only to payloads that are themselves strings: unprocessed snapshot and screenshot label commands become their canonical `SnapshotCommand`/`ScreenshotCommand` siblings with the substituted label, the flat `text` payload is substituted in place via `model_copy`, and struct-payload commands pass through unchanged — payload structs are never traversed. The final `Flow(flow=apps, desc=flow.desc)` constructor runs every existing canonical validator (label pattern, uniqueness, numeric strictness) on the expanded instances, so flow descriptions survive preprocessing and the result is a validated canonical `Flow`. The internal `src/preprocess.py` function raises `pydantic.ValidationError` for unprocessed schema errors and for post-expansion canonical errors, performs no I/O and no device access, and assumes its input is a validating `UnprocessedFlow` instance.

Python users of the unprocessed form get thin builder wrappers mirroring the canonical facade:

```text
lib.unprocessed_flow(apps, desc=None) -> UnprocessedFlow
lib.unprocessed_app_flow(label, bundle, ability, terminate, commands) -> UnprocessedAppFlow
lib.unprocessed_snapshot(label) -> UnprocessedSnapshotCommand
lib.unprocessed_screenshot(label) -> UnprocessedScreenshotCommand
lib.repeat(n_iter, iter_var, commands) -> RepeatMacro
```

`lib.py` exposes `preprocess_flow` as a validating wrapper that mirrors `lib.run()`: it revalidates the input through `UnprocessedFlow.model_validate(flow.model_dump())` before delegating to the internal transformation, so there is a single validation boundary for every caller including the CLI. Input acceptance is enforced statically: the signature takes `UnprocessedFlow` only, so passing a canonical `Flow` or a raw dict is a mypy error, while structurally invalid `UnprocessedFlow` instances fail the wrapper's revalidation at runtime. `lib.py` also exports the builder wrappers and the unprocessed type names `UnprocessedFlow`, `UnprocessedAppFlow`, `Macro`, `RepeatMacro`, `UnprocessedSnapshotCommand`, and `UnprocessedScreenshotCommand`, and adds them to `__all__`. `UnprocessedCommand`, `RepeatMacroPayload`, and payload aliases such as `WaitPayload` remain internal. `lib.run()` continues to accept canonical `Flow` only. `run.py` reads `--flow` JSON into `UnprocessedFlow`, calls `lib.preprocess_flow`, then `lib.run`. There is exactly one transformation path from unprocessed to canonical.

Alternative considered: CLI-only internal loader. Rejected: a public preprocessing API lets Python users author macros programmatically and keeps a single official transformation path.

### Keep canonical Flow as the runtime schema

The existing `Flow`, `AppFlow`, and `Command` models remain the post-expansion representation. The runner and command dispatcher continue to execute only concrete action commands.

Alternative considered: add `repeat` as a real `Command` union member. Rejected because repeat is an authoring macro, not a device action, and would leak macro concerns into runtime execution.

### UnprocessedFlow defers label and uniqueness validation

`UnprocessedAppFlow.label` is a plain string and `UnprocessedFlow` has no uniqueness validator: labels may be placeholder-bearing or duplicated by design in the authored form. The canonical `Flow` model enforces label regexes and uniqueness after expansion. App-label uniqueness that macros cannot affect is still enforced by the canonical model during preprocessing.

## Risks / Trade-offs

- Unprocessed macro JSON is not itself a canonical `Flow` → documentation keeps the two forms distinct and states that result evidence stores what actually executed.
- Pydantic union errors for `Macro | Command` items may be noisy → focused schema-level error tests.
- Users may expect numeric placeholders to work → explicitly documented that substitution is string-only and numeric variation is out of scope.
- Output `flow.json` will not preserve the original macro source → documented; evidence stores what actually executed.
- Unbounded `n_iter` can amplify input size arbitrarily: a ~60-byte macro payload can request ~10^9 expanded commands, tens of GB of dicts, guaranteeing a hang or `MemoryError` before validation. Preprocessing is a pure function and the caller controls input, so this is an accepted, documented risk; a future expansion-count sanity check could raise a clear error above a threshold without imposing a schema bound.
- The `Label`-versus-`str` generic argument is the only difference between canonical and unprocessed snapshot commands → schema test guards against accidental swaps.
- Serialized command JSON key order changes: the `_CommandBase` refactor places `payload` before `action` in model fields, so `model_dump`/`flow.json`/recorder-generated output emit `"payload"` before `"action"`. Existing tests compare dicts (order-insensitive) and the README already permits formatted differences; this is documented so the change is not mistaken for a regression.
- Adding a new command action requires updating the canonical `Command` union, the `UnprocessedCommand` union, and the public facade; the two-form mirror is a maintenance burden accepted for now.
- An item carrying both `action` and `macro` keys is rejected by validation: with `extra="forbid"` on all models, each union member sees the other key as unknown, so the malformed item cannot parse as either form. `{{i}}` in a string payload collapses to a literal `{1}` after substitution, which is typed as-is in free-text payloads and rejected by label validation in labels. No escaping mechanism is provided.
- Flattening the `text` payload is a breaking change for existing flow files using the struct form `{ "text": "..." }` → accepted because the struct form has no macro substitution benefit and the text-command payload shape is unused in practice. The `key` payload intentionally stays a struct: Literal key payloads can never carry a substitutable token, so flattening `key` would break existing flows without enabling macros.
- Future nested loops will need variable scoping → the macro body item type structurally rejects nesting now.

## Verification

The schema mechanics (generic payload base with concrete sibling subclasses, `action` and `macro` discriminators, `Macro | Command` smart union, `n_iter` of zero, empty body, substitution round-trip, and post-expansion canonical validation) were verified in a spike against pydantic 2.13.4 and mypy strict mode.
