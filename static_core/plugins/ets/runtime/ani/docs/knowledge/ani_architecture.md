# ANI Architecture Knowledge

This file only covers ANI runtime architecture, entry tables, native bridges, and release/verify mode boundaries. For reference lifetime, read `ani_reference_lifecycle.md`; for thread state and scopes, read `ani_thread_state_and_scopes.md`.

## Core Model

| Layer | Code anchor | Role | Do not confuse with |
| --- | --- | --- | --- |
| Public ABI | `../../ani.h` | Defines `ani_vm`, `ani_env`, handles, status codes, API tables | Internal ETS objects or verifier wrappers |
| VM API implementation | `../../ani_vm_api.cpp` | Creates/destroys VM, gets env, attaches/detaches threads | Interaction API |
| Interaction API implementation | `../../ani_interaction_api.cpp` | Implements object/class/method/field/reference/scope operations | VerifyANI checker layer |
| Native bridge | `../../ani_helpers.cpp`, `../../arch/` | Marshals ETS native call arguments and selects release/verify/critical entry points | Public C API table |
| VerifyANI wrapper | `../../verify/` | Validates handles/args and calls through to release APIs | Replacement runtime behavior |

## Entry Points

| Entry | Code anchor | Risk |
| --- | --- | --- |
| `GetAniEntryPoint(false)` | `../../ani_helpers.cpp` | Release native call path |
| `GetAniEntryPoint(true)` | `../../ani_helpers.cpp` | VerifyANI native call path |
| `GetAniCriticalEntryPoint()` | `../../ani_helpers.cpp` | Critical native path has stricter object/thread assumptions |
| `ANI_CreateVM` | `../../ani_vm_api.cpp` | Initializes runtime and returns `ani_vm` |
| `PandaAniEnv::Create` | `../../../ets_ani_env.cpp` | Creates env with reference storage; adds verifier only when VM uses VerifyANI |

## Constraints

- Do not change bridge argument order without checking `ani_helpers.cpp`, the affected `arch/*` assembly, and bridge tests.
- Do not bypass `PandaAniEnv` when an operation needs reference storage, pending exception state, or execution context.
- Do not make VerifyANI own real runtime behavior; it validates and delegates to release ANI APIs.
- Do not route API-table changes only through implementation files; `ani.h` is the compatibility source in this directory.

## Modification Checklist

| Question | Required check |
| --- | --- |
| Did the change alter public handles or API-table layout? | Read `ani_public_api_compatibility.md` |
| Did the change alter bridge entry or argument marshaling? | Inspect `ani_helpers.cpp` and matching `arch/` file |
| Did the change affect verify mode? | Inspect corresponding `verify_ani_*` wrapper |
| Did the change touch object references? | Read `ani_reference_lifecycle.md` |

## Code And Tests

| Area | Code | Tests |
| --- | --- | --- |
| Release ANI API | `../../ani_vm_api.cpp`, `../../ani_interaction_api.cpp` | `../../../../tests/ani/tests/*_ops/` |
| Native bridge | `../../ani_helpers.cpp`, `../../arch/` | `../../../../tests/ani/tests/bridges/` |
| VerifyANI mode | `../../verify/` | `../../../../tests/ani/tests/verifyani/` |
