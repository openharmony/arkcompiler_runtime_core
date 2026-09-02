## 1. Schema and API

- [x] 1.1 Add optional `"$desc"` alias-backed strict string metadata to the canonical `Flow` model.
- [x] 1.2 Ensure canonical flow JSON serialization emits `"$desc"` when provided and omits it when absent.
- [x] 1.3 Update `lib.flow()` to accept optional `desc=None` while preserving existing `lib.flow(apps)` compatibility.
- [x] 1.4 Ensure runner/device execution ignores description metadata.

## 2. Tests

- [x] 2.1 Add schema tests for valid `"$desc"`, absent `"$desc"`, and invalid non-string `"$desc"`.
- [x] 2.2 Add public facade tests for `lib.flow(..., desc="...")`.
- [x] 2.3 Add canonical output tests proving CLI or runner output `flow.json` preserves `"$desc"`.
- [x] 2.4 Add or update type-check fixture coverage for the optional `desc` parameter if needed.

## 3. Documentation

- [x] 3.1 Update README flow JSON documentation with optional `"$desc"` usage.
- [x] 3.2 Update programmatic API documentation for `lib.flow(apps, desc=None)`.

## 4. Verification

- [x] 4.1 Run the project test suite.
- [x] 4.2 Run lint/typecheck commands used by the project.
- [x] 4.3 Run `openspec validate "add-flow-description" --type change`.
- [x] 4.4 Run `openspec validate --all`.
