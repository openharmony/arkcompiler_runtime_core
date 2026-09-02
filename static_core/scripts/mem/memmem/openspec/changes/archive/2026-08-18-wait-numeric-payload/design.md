## Context

The `wait` command payload is defined in `src/schema.py:17` as `WaitPayload = Annotated[float, Field(ge=0, strict=True)]` — it already accepts non-negative integers and floats, and `execute_wait(payload: float)` in `src/commands.py` sleeps the requested seconds. The benchmark-flow spec, however, describes the payload as a "non-negative integer" and its tests only reject a string `"1"` and `-1`, leaving float acceptance untested and the spec at odds with behavior. Separately, the flat `text` payload breaking change (struct → flat string) has no regression test rejecting the legacy struct form.

## Goals / Non-Goals

**Goals:**
- Align the benchmark-flow spec with actual behavior: `wait` payload is a non-negative number (integer or float).
- Add tests: float `wait` accepted; negative float and non-numeric `wait` rejected; legacy struct `text` payload rejected.

**Non-Goals:**
- No production code changes — schema and runtime already behave correctly.
- No runner/device-layer changes (`time.sleep` already accepts floats).

## Decisions

- **Spec-first alignment, no code change.** The implementation already matches the intended behavior; the change is spec wording (`non-negative integer` → `non-negative numeric payload (integer or float)`), a new rejection scenario in benchmark-flow, and schema test additions. Decision: keep `strict=True` so strings are rejected rather than coerced; float/int both pass `ge=0`.
- **Tests live in `SchemaTest` near existing wait tests.** Add `test_wait_payload_accepts_float` (e.g. `2.5`), `test_wait_payload_rejects_negative_float` (`-0.5`), and `test_text_payload_rejects_struct_form` (`{"action": "text", "payload": {"text": "x"}}`). Existing tests (`test_wait_payload_must_be_non_negative_integer`, `test_wait_payload_must_be_integer`) remain.
- **testing-support delta uses ADDED requirement** ("Schema regression coverage...") rather than extending the large "Programmatic interface tests" requirement — new test-coverage concern, no existing behavior changes.

## Risks / Trade-offs

- [Float waits could conceal integer-only expectations downstream] → No downstream code depends on integer waits; runner sleeps by seconds.
- [Spec text and README divergence] → README already shows fractional waits (`payload: 2.1`); no README change needed.
- [Legacy struct text payload silently accepted once more] → New rejection test guards the flat-payload contract.