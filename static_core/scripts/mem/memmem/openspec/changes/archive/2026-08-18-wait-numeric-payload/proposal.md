## Why

The `wait` payload is documented as a non-negative integer, but the schema and runtime already accept non-negative floats (e.g. `2.1` seconds) and the README shows fractional waits. The spec, tests, and code disagree, and the legacy struct form of the `text` payload is not covered by a rejection test.

## What Changes

- `wait` payload accepts a non-negative numeric value: integer or float seconds (code already supports it; spec and tests will be aligned).
- Negative values and non-numeric values (e.g. strings) remain rejected.
- Add a regression test asserting the legacy struct form `{"action": "text", "payload": {"text": ...}}` is rejected for the flat `text` action.

## Capabilities

### New Capabilities

- (none)

### Modified Capabilities

- `benchmark-flow`: "Commands execute with action-specific semantics" requirement — `wait` payload is a non-negative integer *or float* number of seconds.
- `testing-support`: "Programmatic interface tests cover public facade behavior" requirement — test suite verifies non-negative numeric `wait` payloads are accepted (integer and float) and rejects negative/non-numeric values; verifies the flat `text` payload rejects the legacy struct form.

## Impact

- `openspec/specs/benchmark-flow/spec.md`: scenario wording for `wait` payload type.
- `openspec/specs/testing-support/spec.md`: new test-coverage scenarios.
- `test/schema_test.py`: new tests (no production code change required — `WaitPayload` already accepts floats).
- README already documents fractional waits; no change needed.