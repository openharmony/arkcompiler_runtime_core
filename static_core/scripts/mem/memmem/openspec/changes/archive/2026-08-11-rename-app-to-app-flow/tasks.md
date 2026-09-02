## 1. Public API Rename

- [x] 1.1 Rename `lib.app()` to `lib.app_flow()` while preserving the existing function signature and validation behavior.
- [x] 1.2 Update `lib.__all__` so `app_flow` is public and `app` is not exported.
- [x] 1.3 Ensure no compatibility alias for `lib.app()` remains.

## 2. Documentation and Tests

- [x] 2.1 Update README programmatic API examples to use `lib.app_flow()`.
- [x] 2.2 Update public facade tests to construct flows with `lib.app_flow()`.
- [x] 2.3 Update public facade export tests to expect `app_flow` and reject `app`.
- [x] 2.4 Search the repository for remaining `lib.app()` references and update intended public API usage.

## 3. Verification

- [x] 3.1 Run focused programmatic API tests.
- [x] 3.2 Run type checking and full tests.
- [x] 3.3 Validate the OpenSpec change.
