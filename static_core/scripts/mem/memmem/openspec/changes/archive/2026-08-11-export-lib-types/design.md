## Context

`lib.py` is the documented public module for user scripts. Builder functions already carry precise return and argument types from `src.schema`, `src.device`, and `src.hdc`, but the public facade's `__all__` only includes functions. Typed users may need to annotate local helpers or variables as `lib.Flow`, `lib.AppFlow`, `lib.Command`, or `lib.Device` without importing `src.*`.

Under stricter mypy settings, relying on implementation imports that are not explicit public exports is fragile. The public contract should include the type names that appear in public function signatures and common typed-script annotations.

## Goals / Non-Goals

**Goals:**
- Export public type names from `lib.py` for flow, command, device, and HDC types used by the public API.
- Keep the public facade as the only documented import surface for user scripts.
- Keep internal runner/configuration types out of the public facade.
- Add mypy-oriented tests or fixtures proving a user script can type-check with `lib.*` types.

**Non-Goals:**
- Creating new wrapper classes around existing schema/device types.
- Moving models out of `src.schema`.
- Exporting internal orchestration types such as `BenchmarkOptions` or result-store helpers.

## Decisions

### Re-export existing classes and aliases directly

`lib.py` should publicly export the same type objects it already uses in function annotations. This keeps runtime identity intact: `lib.Flow` is the same class as the validated flow model, and `lib.Command` is the same command union used by `AppFlow.commands`.

Alternative considered: define duplicate public protocols or aliases with different names. That would add indirection and risk divergence from the actual schema models.

### Export API-relevant flow, command, and handle types

The minimum set for annotations is `Flow`, `AppFlow`, `Command`, `Device`, and `Hdc`. Builder functions also return specific command classes, so command model types such as `TapCommand` and `WaitCommand` should be public. Payload structs are not part of the public API and should remain unexported.

Alternative considered: export payload model types as well. This was rejected because public builders accept primitive arguments and hide payload construction details from user scripts.

### Keep internal implementation types private

`BenchmarkOptions`, result stores, report generation, and runner entrypoints remain internal because user scripts should call `lib.run()` rather than orchestrating internals.

## Risks / Trade-offs

- Larger public surface area. → Limit exports to type names that appear in public signatures or are schema types returned by public builders.
- Future schema renames become public API changes. → This is acceptable because typed users need stable names; future changes should go through OpenSpec.
- Users may import schema types and construct models directly. → Accept because they are the actual validated models and still avoid `src.*` imports.
