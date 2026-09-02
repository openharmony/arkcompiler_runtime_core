## Why

Reviewers need a human-readable explanation of what a benchmark flow is intended to measure without relying on external notes. Adding an optional top-level description keeps this context next to the flow and preserved in canonical run evidence.

## What Changes

- Add optional top-level `"$desc"` string metadata to the canonical `Flow` model and CLI `flow.json` schema.
- Preserve `"$desc"` in output `flow.json` when provided.
- Keep runner behavior unchanged: `"$desc"` is metadata only and does not affect execution.
- Extend the public `lib.flow()` helper with optional `desc=None` support.
- Existing flows without `"$desc"` remain valid.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: Flow JSON accepts optional top-level `"$desc"` string metadata.
- `programmatic-interface`: Public `lib.flow()` can construct a flow with optional description metadata.
- `result-evidence`: Canonical output `flow.json` preserves the flow description metadata when present.
- `testing-support`: Tests cover schema validation, public builder support, and canonical output preservation for flow descriptions.

## Impact

- Affected code: `src/schema.py`, `lib.py`, README documentation, and schema/lib/result evidence tests.
- No device, runner, output layout, or command execution behavior changes.
- No breaking change to existing valid flow files or user scripts that call `lib.flow(apps)`.
