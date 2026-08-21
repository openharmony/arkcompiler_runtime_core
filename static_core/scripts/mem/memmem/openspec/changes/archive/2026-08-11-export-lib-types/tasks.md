## 1. Public Type Exports

- [x] 1.1 Add public type names to `lib.__all__` for flow, app-flow, command, HDC, and device types used by the public API.
- [x] 1.2 Ensure payload structs and internal orchestration types such as `BenchmarkOptions` remain outside `lib.__all__`.
- [x] 1.3 Keep existing builder function signatures and runtime behavior unchanged.

## 2. Typed Usage Tests

- [x] 2.1 Update public facade export tests to expect exported public type names.
- [x] 2.2 Add a mypy-checked fixture or test case that imports `lib` and annotates user helpers with `lib.Flow`, `lib.AppFlow`, `lib.Command`, `lib.Device`, and `lib.Hdc`.
- [x] 2.3 Ensure the typed fixture does not import documented APIs from `src.*`.

## 3. Documentation

- [x] 3.1 Update README programmatic API documentation with a small typed helper example using public `lib.py` type names.

## 4. Verification

- [x] 4.1 Run focused public facade and mypy typed-usage tests.
- [x] 4.2 Run type checking and full tests.
- [x] 4.3 Validate the OpenSpec change.
