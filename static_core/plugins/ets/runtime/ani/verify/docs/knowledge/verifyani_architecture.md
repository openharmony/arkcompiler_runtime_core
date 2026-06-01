# VerifyANI Architecture Knowledge

This file only covers VerifyANI wrapper architecture, state ownership, and release API call-through boundaries. For checker rules, read `verifyani_checker_rules.md`; for error reporting, read `verifyani_error_reporting.md`.

## Core Model

| Component | Code anchor | Owns | Must not own |
| --- | --- | --- | --- |
| `VEnv` | `../../types/venv.*` | Verified env API table and owner release `ani_env` pointer | Real `PandaAniEnv` storage |
| `EnvANIVerifier` | `../../env_ani_verifier.*` | Per-env native frames, local scopes, local verified refs | VM-wide global refs |
| `ANIVerifier` | `../../ani_verifier.*` | Global/weak refs, methods, fields, resolvers, report hooks | Local frame lifetime |
| `VRef` subclasses | `../../types/vref.*` | Typed wrapper over release `ani_ref` | Object memory |
| `VWRef` | `../../types/vwref.*` | Typed wrapper over release `ani_wref` | Strong object lifetime |
| `VMethod`/`VField` | `../../types/vmethod.*`, `../../types/vfield.*` | Verified handle identity | Runtime method/field allocation |

## Mode Switch

| Step | Code anchor | Rule |
| --- | --- | --- |
| VM configured for VerifyANI | `../../../ani_options*`, VM state | `--verify:ani` enables verifier state |
| Env created | `../../../../ets_ani_env.cpp` | `PandaAniEnv` creates `EnvANIVerifier` when VM is verify-enabled |
| Public env passed to caller | `VEnv` | `c_api` points to verify interaction API |
| Verify wrapper executes | `../../verify_ani_interaction_api.cpp` | Validate verified args, unwrap handles, call release API |
| Release env access | `PandaAniEnv::FromAniEnv` | Must unwrap `VEnv` to owner env |

## State Ownership

| State | Owner | Cleanup trigger |
| --- | --- | --- |
| Local verified refs | `EnvANIVerifier::frames_` | Native frame or local scope pop |
| Global verified refs | `ANIVerifier::GlobalData::grefsMap` | Explicit global ref delete |
| Weak verified refs | `ANIVerifier::GlobalData::wrefsMap` | Explicit weak ref delete |
| Method/field verified handles | `ANIVerifier` method/field maps | Verifier-owned delete path |
| Resolver verified handles | `ANIVerifier` resolver map | Resolver delete/settle path |

## Constraints

- Do not pass `VEnv` directly to code that expects a real `PandaAniEnv`; use `PandaAniEnv::FromAniEnv` or `VEnv::GetEnv()`.
- Do not store local verified refs in `ANIVerifier::GlobalData`.
- Do not create verified refs without registering them in the correct verifier owner.
- Do not expose `V*` wrapper types through public `ani.h`.
- Keep `IsWorkaroundNoCrashIfInvalidUsage` private and treat it as verifier policy only.

## Modification Checklist

| Question | Required check |
| --- | --- |
| Does this wrapper call the release API? | Ensure it unwraps verified handles first |
| Is the handle local/global/weak/method/field/resolver? | Use the correct verifier owner map or frame |
| Does this change API parity? | Compare release `ani_interaction_api.cpp` or `ani_vm_api.cpp` |
| Does this change native frame behavior? | Check `EnvANIVerifier::PushNativeFrame`/`PopNativeFrame` |

## Code And Tests

| Area | Code | Tests |
| --- | --- | --- |
| Verify interaction API | `../../verify_ani_interaction_api.cpp` | `../../../../../tests/ani/tests/verifyani/*_ops/` |
| Verify VM API | `../../verify_ani_vm_api.cpp` | `../../../../../tests/ani/tests/verifyani/vm_ops/` |
| Verified refs/scopes | `../../env_ani_verifier.*`, `../../types/*` | `../../../../../tests/ani/tests/verifyani/ref_ops/`, `../../../../../tests/ani/tests/verifyani/scope_ops/` |
