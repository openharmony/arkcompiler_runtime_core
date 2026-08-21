## Context

The canonical `Flow` model currently contains only the executable app-flow list. Reviewers may need a short human-readable explanation of what a flow is intended to measure, and canonical output `flow.json` already preserves validated flow metadata.

## Goals / Non-Goals

**Goals:**
- Add optional top-level `"$desc"` metadata to canonical `Flow`.
- Preserve the description in CLI and programmatic canonical output.
- Expose optional description construction through `lib.flow()`.
- Keep existing flows and scripts compatible.

**Non-Goals:**
- Adding per-AppFlow descriptions.
- Changing command execution, runner behavior, result paths, or report generation.
- Interpreting, parsing, or rendering the description outside canonical JSON preservation.

## Decisions

### Use `"$desc"` as the JSON field name

The field is named `"$desc"` to make it clearly metadata rather than executable benchmark flow content. In Python, the canonical model can expose a normal attribute such as `desc` with Pydantic alias `"$desc"` so serialized canonical JSON uses the requested field name.

### Keep it optional and strict string

Existing flows without `"$desc"` remain valid. When present, the value must be a string. Empty strings are not explicitly rejected unless standard strict string validation fails.

### Preserve through canonical serialization

`flow.model_dump_json(indent=2)` should emit `"$desc"` when provided. The runner continues to ignore the field operationally.

### Extend `lib.flow()` compatibly

`lib.flow(apps)` remains valid. `lib.flow(apps, desc="...")` creates a `Flow` with description metadata.

## Risks / Trade-offs

- Pydantic alias behavior could accidentally serialize `desc` instead of `"$desc"` → cover with canonical output tests.
- The `$` prefix is unusual in Python attribute names → use an alias and keep the public helper parameter as `desc`.
