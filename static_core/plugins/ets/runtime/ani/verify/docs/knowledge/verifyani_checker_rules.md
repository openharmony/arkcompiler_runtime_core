# VerifyANI Checker Rules Knowledge

This file only covers adding, composing, and testing VerifyANI checkers. For the VerifyANI state model, read `verifyani_architecture.md`; for error reporting, read `verifyani_error_reporting.md`.

## Core Model

| Piece | Code anchor | Role |
| --- | --- | --- |
| Checker list | `../../verify_ani_checker.h` `ANI_VERIFICATION_MAP` | Declares available verifier checks |
| Argument type list | `../../verify_ani_checker.h` `ANI_ARG_TYPES_MAP` | Maps logical checker argument names to C++ types |
| Wrapper macro | `../../verify_ani_interaction_api.cpp` `VERIFY_ANI_ARGS` | Runs checker set for current API wrapper |
| Abort helper | `../../verify_ani_interaction_api.cpp` `VERIFY_ANI_ABORT_IF_ERROR` | Reports optional string error and returns `ANI_ERROR` |
| Checker implementation | `../../verify_ani_checker.cpp` | Implements specific validation logic |

## Checker Boundaries

| Check type | Should validate | Should not validate |
| --- | --- | --- |
| Env/VM check | Verified wrapper identity and current state | Real runtime behavior already handled by release API |
| Ref check | Wrapper validity, scope validity, ref kind | Object semantic type unless API requires it |
| Method/field check | Verified handle validity and expected static/instance kind | Re-resolve symbols unnecessarily |
| Storage check | Output pointer validity | Allocate result objects before release API succeeds |
| Primitive check | ANI primitive domain, e.g. boolean value | Convert or clamp caller input silently |
| Args check | Variadic/value-array shape and ref arguments | Mutate original args before validation completes |

## Add Or Change A Checker

| Step | Required action |
| --- | --- |
| 1 | Find the release API behavior in `../../../ani_interaction_api.cpp` or `../../../ani_vm_api.cpp` |
| 2 | Decide whether validation belongs in a reusable checker or wrapper-local precondition |
| 3 | Add enum/macro entry only if a reusable checker is needed |
| 4 | Implement validation in `verify_ani_checker.cpp` with precise failure reason |
| 5 | Wire wrapper through `VERIFY_ANI_ARGS` or explicit `VERIFY_ANI_ABORT_IF_ERROR` |
| 6 | Add focused negative tests under `../../../../../tests/ani/tests/verifyani/` |
| 7 | Run focused `ani_verify_test_*_gtests` and one nearby related target |

## Constraints

- Do not add a checker that duplicates a release API side effect; checkers should validate before call-through.
- Do not accept release ANI handles where a verified wrapper is required, except in explicit unwrap/cast code.
- Do not silently coerce invalid `ani_boolean`, invalid refs, or mismatched method args.
- Do not add broad checks that make unrelated APIs fail before their documented release preconditions.

## Test Anchors

| API family | Test directory |
| --- | --- |
| VM/env | `../../../../../tests/ani/tests/verifyani/vm_ops/`, `../../../../../tests/ani/tests/verifyani/env_ops/` |
| refs/scopes | `../../../../../tests/ani/tests/verifyani/ref_ops/`, `../../../../../tests/ani/tests/verifyani/scope_ops/`, `../../../../../tests/ani/tests/verifyani/local_scope_ops/` |
| object/class/function | `../../../../../tests/ani/tests/verifyani/object_ops/`, `../../../../../tests/ani/tests/verifyani/class_ops/`, `../../../../../tests/ani/tests/verifyani/function_ops/` |
| arrays/strings/tuple/any | `../../../../../tests/ani/tests/verifyani/array_ops/`, `../../../../../tests/ani/tests/verifyani/string_ops/`, `../../../../../tests/ani/tests/verifyani/tuplevalue/`, `../../../../../tests/ani/tests/verifyani/any_ops/` |
| generated primitive/variable tests | `../../../../../tests/ani/tests/verifyani/primitive_box_unbox/`, `../../../../../tests/ani/tests/verifyani/variable_ops/` |

## Modification Checklist

| Question | Required check |
| --- | --- |
| Is this a reusable validation concept? | Add to checker map only if reused or central |
| Does the wrapper still call the release API exactly once on success? | Inspect wrapper control flow |
| Does the failure return `ANI_ERROR` or another status intentionally? | Match existing wrapper pattern |
| Are negative tests focused on the invalid input? | Avoid tests that depend on unrelated runtime failure |
