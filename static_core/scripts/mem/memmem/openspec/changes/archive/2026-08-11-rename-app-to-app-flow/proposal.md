## Why

The public programmatic helper `lib.app()` constructs an AppFlow, not an application object. Renaming it to `lib.app_flow()` makes the API clearer and aligns the helper name with the schema concept it creates.

## What Changes

- **BREAKING**: Rename the public programmatic builder `lib.app()` to `lib.app_flow()`.
- Remove `app` from the documented/exported public facade and export `app_flow` instead.
- Update documentation and tests that construct flows through `lib.py`.

## Capabilities

### New Capabilities

### Modified Capabilities
- `programmatic-interface`: public app-flow builder helper is renamed from `app()` to `app_flow()`.
- `testing-support`: public facade tests construct flows with `lib.app_flow()` and assert `lib.app()` is not public.

## Impact

- Affected code: `lib.py`, README programmatic examples, and public facade tests.
- Public API impact: existing user scripts that call `lib.app()` must switch to `lib.app_flow()`.
- No output format, CLI behavior, or benchmark runner behavior changes.
