## Why

Typed user scripts need names such as `lib.Flow`, `lib.AppFlow`, `lib.Command`, `lib.Device`, and `lib.Hdc` for annotations when running mypy. The public facade currently exposes builder functions but intentionally omits these type names from `__all__`, which makes typed usage unclear and can fail under stricter mypy/export settings.

## What Changes

- Export public type names from `lib.py` for the flow models, command models, and device/HDC handle types used by the public API.
- Keep internal orchestration types, especially `BenchmarkOptions`, out of the public facade.
- Document how typed user scripts can annotate helper functions without importing `src.*`.
- Add tests that validate public type names are exported and usable for mypy-checked user code.

## Capabilities

### New Capabilities

### Modified Capabilities
- `programmatic-interface`: public `lib.py` facade includes type names needed by mypy-checked user scripts.
- `testing-support`: public facade tests cover exported type names and typed user-script compatibility.

## Impact

- Affected code: `lib.py`, README, and public facade tests.
- Public API impact: additive; existing builder and runner calls remain unchanged.
- No CLI, output format, benchmark execution, or dependency changes.
