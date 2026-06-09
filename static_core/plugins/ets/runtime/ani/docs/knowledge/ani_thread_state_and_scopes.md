# ANI Thread State And Scope Knowledge

This file only covers ANI thread attachment, managed/native state transitions, native frames, and local-scope boundaries. For reference lifetime, read `ani_reference_lifecycle.md`.

## Core Model

| State or action | Code anchor | Meaning | Risk |
| --- | --- | --- | --- |
| VM creation | `../../ani_vm_api.cpp` `ANI_CreateVM` | Creates runtime and returns current VM | Runtime options and logger behavior are externally observable |
| Get env | `../../ani_vm_api.cpp` `GetEnv` | Requires attached mutator and current execution context | Returning env for unattached thread is invalid |
| Attach thread | `../../ani_vm_api.cpp` `AttachCurrentThread` | Attaches OS/FFRT worker and returns env | Double attach must fail |
| Detach thread | `../../ani_vm_api.cpp` | Leaves current VM/thread context | Must not leave VerifyANI frames dangling |
| Managed access | `../../scoped_objects_fix.h` `ScopedManagedCodeFix` | Enters managed code when native API touches ETS objects | Missing transition can break GC/runtime invariants |
| Native frame | `../../ani_helpers.cpp`, `../../verify/env_ani_verifier.*` | Bridge-created call frame for native invocation | VerifyANI ref validity depends on frame stack |

## Thread-State Rules

| Rule | Reason |
| --- | --- |
| Must verify current thread is attached before returning `ani_env` | `ani_env` is tied to `EtsExecutionContext` |
| Must not call managed object access without `ScopedManagedCodeFix` or an equivalent managed state guard | ETS object access expects managed-state invariants |
| Must preserve pending-exception checks in env-facing APIs | ANI returns `ANI_PENDING_ERROR` instead of continuing through invalid state |
| Must treat FFRT attach behavior as thread-affinity sensitive | FFRT task migration is explicitly disabled in attach path |

## Scope Interaction

| Scope | Release owner | VerifyANI owner | Notes |
| --- | --- | --- | --- |
| Native frame | Bridge/runtime call boundary | `EnvANIVerifier::PushNativeFrame` / `PopNativeFrame` | Local verified refs are frame-scoped |
| Local scope | `EtsReferenceStorage` local frame | `EnvANIVerifier::CreateLocalScope` | Capacity must be nonzero |
| Escape local scope | `EtsReferenceStorage` local frame with one escaped ref | `EnvANIVerifier::CreateEscapeLocalScope` | Escaped ref must be valid in the current scope |

## Modification Checklist

| Question | Required check |
| --- | --- |
| Does this API require an attached thread? | Check `Mutator::GetCurrent()` and current execution context handling |
| Does it touch ETS objects? | Use `ScopedManagedCodeFix` or prove the caller is already in a valid managed state |
| Does it run in VerifyANI mode? | Check `PandaAniEnv::IsVerifyANI()` and `EnvANIVerifier` frame behavior |
| Does it change attach/detach? | Run VM/thread tests and VerifyANI VM tests |

## Code And Tests

| Area | Code | Tests |
| --- | --- | --- |
| VM/thread APIs | `../../ani_vm_api.cpp` | `../../../../tests/ani/tests/vm_ops/`, `../../../../tests/ani/tests/verifyani/vm_ops/` |
| Managed transition | `../../scoped_objects_fix.h`, `../../ani_helpers.cpp` | Nearby operation tests that touch ETS objects |
| Scope APIs | `../../ani_interaction_api.cpp`, `../../verify/env_ani_verifier.*` | `../../../../tests/ani/tests/verifyani/scope_ops/`, `../../../../tests/ani/tests/verifyani/local_scope_ops/` |
