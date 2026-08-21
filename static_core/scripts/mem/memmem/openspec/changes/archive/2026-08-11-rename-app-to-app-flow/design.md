## Context

`lib.py` is the documented public programmatic API. It currently exposes `lib.app()` as a thin wrapper around the `AppFlow` schema model. The name is short, but it can be confused with an application descriptor rather than an AppFlow builder that includes commands and termination behavior.

## Goals / Non-Goals

**Goals:**
- Rename the public builder from `app()` to `app_flow()`.
- Keep the builder as a thin wrapper over the existing `AppFlow` schema model.
- Update documentation and tests so the public facade consistently uses `app_flow()`.

**Non-Goals:**
- Changing the underlying JSON schema or `AppFlow` model name.
- Changing CLI flow files or benchmark execution behavior.
- Keeping `lib.app()` as a compatibility alias.

## Decisions

### Make the rename breaking and remove the old export

The public facade should export `app_flow` and stop exporting `app`. This avoids maintaining two names for the same public concept and keeps examples unambiguous.

Alternative considered: keep `app` as a backwards-compatible alias. This would reduce migration friction but preserve the ambiguity that motivated the rename.

### Keep signatures and validation unchanged

`app_flow(label, bundle, ability, terminate, commands)` should preserve the existing explicit fields and validation behavior of `app(...)`. Only the public helper name changes.

Alternative considered: reshape the function signature while renaming. That is unnecessary because the current explicit fields match the schema and existing API design.

## Risks / Trade-offs

- Existing scripts using `lib.app()` will break. → Document the rename and update examples/tests to use `lib.app_flow()`.
- The schema type remains `AppFlow`, which differs from the helper name only by style. → Keep model names unchanged to avoid broader schema churn.
