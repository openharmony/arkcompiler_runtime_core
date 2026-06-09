# ANI Reference Lifecycle Knowledge

This file only covers ANI references, scopes, null/undefined handling, and GC-visibility boundaries. For thread state, read `ani_thread_state_and_scopes.md`.

## Core Model

| Concept | Code anchor | Lifetime owner | Common misuse |
| --- | --- | --- | --- |
| `ani_ref` local ref | `../../scoped_objects_fix.h` `AddLocalRef` | Current env reference storage and local frame | Storing it in VM-wide state |
| `ani_ref` global ref | `../../scoped_objects_fix.h` `AddGlobalRef` | Global object storage through `EtsReferenceStorage` | Deleting it with local-ref logic |
| `ani_wref` weak ref | `../../scoped_objects_fix.h` `AddWeakRef`, `GetLocalRef` | Weak reference storage; object may be released | Treating `nullptr` result as a live object |
| Undefined ref | `ManagedCodeAccessor::GetUndefinedRef` | Sentinel `EtsReference::GetUndefined()` | Treating it as `nullptr` |
| Null ref | `ManagedCodeAccessor::GetNullRef` | Local ref to runtime null object | Treating it as undefined |
| Escape local scope | `DestroyEscapeLocalScope` | Pops frame and re-adds one escaped local ref | Returning a ref not owned by the scope |

## Boundary Rules

| Rule | Reason |
| --- | --- |
| Must create/delete refs through `ManagedCodeAccessor` or `ScopedManagedCodeFix` | Keeps `EtsReferenceStorage` ownership and GC tracking consistent |
| Must check ref kind before delete | Local/global/weak refs use different invariants |
| Must preserve undefined fast paths | Undefined is a sentinel, not an allocated object |
| Must convert weak ref to local ref before object use | Weak target may have been released |
| Must not store local refs beyond their frame/scope | Local frame pop invalidates them |

## Scope And Ref Operations

| Operation | Code anchor | Required guardrail |
| --- | --- | --- |
| Ensure local capacity | `EnsureLocalEnoughRefs` | Reject zero and overflow-sized requests |
| Create local scope | `CreateLocalScope` | Push a local frame with requested capacity |
| Destroy local scope | `DestroyLocalScope` | Pop current local frame without escaping a ref |
| Create escape scope | `CreateEscapeLocalScope` | Push frame intended for one escaped ref |
| Destroy escape scope | `DestroyEscapeLocalScope` | Release helper preserves the undefined sentinel; otherwise the escaped ref must be local |
| Delete local/global/weak ref | `DelLocalRef`, `DelGlobalRef`, `DelWeakRef` | Verify ref kind before removal |

VerifyANI wrapper-reference ownership is covered by `../../verify/docs/knowledge/verifyani_architecture.md`; keep release reference semantics and VerifyANI validation aligned when changing shared reference APIs.

## Modification Checklist

| Question | Required check |
| --- | --- |
| Can this ref outlive the current native frame? | It must be global, weak, or explicitly escaped |
| Can this ref be undefined or null? | Handle both separately |
| Is a weak ref dereferenced? | Use weak-to-local conversion and check released flag |
| Does VerifyANI need matching scope/ref validation? | Check `../../verify/env_ani_verifier.*`, `../../verify/types/*`, and `../../verify/docs/knowledge/verifyani_architecture.md` |

## Code And Tests

| Area | Code | Tests |
| --- | --- | --- |
| Release ref/scope | `../../scoped_objects_fix.h`, `../../ani_interaction_api.cpp` | `../../../../tests/ani/tests/ref_ops/`, `../../../../tests/ani/tests/wref_ops/` |
| Verify ref/scope | `../../verify/env_ani_verifier.*`, `../../verify/types/vref.*`, `../../verify/types/vwref.*` | `../../../../tests/ani/tests/verifyani/ref_ops/`, `../../../../tests/ani/tests/verifyani/scope_ops/`, `../../../../tests/ani/tests/verifyani/local_scope_ops/` |
