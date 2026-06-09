# ANI Public API Compatibility Knowledge

This file only covers compatibility boundaries for `ani.h` and public ANI behavior. For implementation architecture, read `ani_architecture.md`; for reference and thread rules, read the related knowledge files.

## Public Surface

| Surface | Code anchor | Compatibility risk |
| --- | --- | --- |
| Version macro | `../../ani.h` `ANI_VERSION_1` | Version negotiation and `ANI_INVALID_VERSION` behavior |
| Primitive typedefs | `../../ani.h` | ABI size/sign mismatch |
| Reference class/typedef hierarchy | `../../ani.h` | C++ type relationship and C opaque-handle contract |
| `ani_status` enum | `../../ani.h` | Numeric/order changes can break callers |
| `ani_option`, `ani_options` | `../../ani.h` | VM creation option ABI |
| `__ani_vm_api`, `__ani_interaction_api` | `../../ani.h` | Function pointer table layout |
| `ANI_CreateVM`, `ANI_GetCreatedVMs` | `../../ani_vm_api.cpp` | Extern "C" entry points |

## Constraints

- Do not reorder, remove, or repurpose enum values in `ani_status` without explicit compatibility approval.
- Do not reorder function pointers in public API tables without treating the change as ABI-breaking.
- Do not expose `PandaAniEnv`, `EtsObject`, `EtsReference`, `VEnv`, `VRef`, or other internal classes through `ani.h`.
- Do not change public API semantics only in release mode or only in VerifyANI mode.
- Must synchronize public interface changes in `../../ani.h` with the OpenHarmony external interface repository before claiming the change is complete.

## Interface Synchronization

ANI is part of the OpenHarmony ecosystem. If a change modifies any public interface in `../../ani.h`, it must also be synchronized with:

| External repository | External file |
| --- | --- |
| [`interface_sdk_c`](https://gitcode.com/openharmony/interface_sdk_c) | `ani/ani.h` |

## API Parity Checklist

| If changing | Also inspect |
| --- | --- |
| VM API function | `../../ani_vm_api.cpp`, `../../verify/verify_ani_vm_api.cpp`, `../../verify/verify_ani_vm_api.h` |
| Interaction API function | `../../ani_interaction_api.cpp`, `../../verify/verify_ani_interaction_api.cpp`, `../../verify/verify_ani_interaction_api.h` |
| New handle type | `../../ani.h`, `../../scoped_objects_fix.h`, `../../verify/types/`, `../../verify/verify_ani_checker.h` |
| New status code | All callers comparing `ani_status`, release tests, VerifyANI tests |
| New option | `../../ani_options*`, `../../ani_vm_api.cpp`, option tests |

## Modification Checklist

| Question | Required answer |
| --- | --- |
| Is the signature visible in `ani.h`? | Treat as public API |
| Does the API table layout change? | Require release and VerifyANI registration review |
| Does behavior change for invalid args or pending exception? | Add or update release and VerifyANI negative tests |
| Is external interface sync required? | If `../../ani.h` changed, synchronize `interface_sdk_c:ani/ani.h`; otherwise state why it is not required |

## Code And Tests

| Area | Code | Tests |
| --- | --- | --- |
| Public header | `../../ani.h` | Build all ANI tests; inspect external interface sync requirement |
| VM/options | `../../ani_vm_api.cpp`, `../../ani_options*` | `../../../../tests/ani/tests/vm_ops/`, `../../../../tests/ani/tests/options/` |
| Interaction APIs | `../../ani_interaction_api.cpp` | `../../../../tests/ani/tests/*_ops/` |
| Verify parity | `../../verify/` | `../../../../tests/ani/tests/verifyani/` |
