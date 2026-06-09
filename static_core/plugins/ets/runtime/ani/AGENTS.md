# ANI Agent Guide

## Code Map

This AGENTS.md applies to `runtime_core/static_core/plugins/ets/runtime/ani/`.

ANI is the Ark Native Interface runtime layer for native C/C++ code to create or attach to an ETS VM, obtain `ani_env`, operate ArkTS objects, call methods, manage references, and cross the managed/native boundary. The most important boundary is: public ANI handles and API tables must stay stable while internal ETS runtime objects, reference storage, and VerifyANI wrappers remain implementation details.

Key areas:

- `ani.h`: public C/C++ ABI surface, handle typedefs, status codes, VM API table, interaction API table.
- `ani_vm_api.cpp`: `ANI_CreateVM`, VM lifecycle, thread attach/detach, `ani_vm` API implementation.
- `ani_interaction_api.cpp`: object/class/method/field/string/array/reference/scope operations.
- `scoped_objects_fix.h`: `ScopedManagedCodeFix`, `ManagedCodeAccessor`, local/global/weak reference conversion.
- `ani_helpers.cpp`, `arch/`: native bridge entry points and argument marshaling between ETS and native code.
- `ani_checkers.h`, `ani_type_check.h`, `ani_type_info.h`, `ani_converters.h`: non-VerifyANI checks and conversion helpers.
- `verify/`: VerifyANI API wrappers, verifier state, checked handles, checker dispatch. Read `verify/AGENTS.md` before editing this subtree.
- `docs/knowledge/`: task-specific agent knowledge. These files are guardrails and routing maps, not tutorials.
- `../../tests/ani/`: ANI test files and CMake test target definitions.

## Knowledge Routing

Use the table below as keyword-based routing before planning or editing.

| Keywords | Read |
| --- | --- |
| ANI architecture / mode switch / API table / bridge / `AniEntryPoint` / `AniVerifyEntryPoint` / `AniCriticalNativeEntryPoint` / `arch/` / `ani_helpers.cpp` | `docs/knowledge/ani_architecture.md` |
| `ani_ref` / `ani_wref` / local ref / global ref / weak ref / null / undefined / nullish / scope / escape scope / `scoped_objects_fix.h` | `docs/knowledge/ani_reference_lifecycle.md` |
| thread attach / thread detach / `ANI_CreateVM` / `GetEnv` / `ScopedManagedCodeFix` / `ManagedCodeBegin` / `ManagedCodeEnd` / native frame | `docs/knowledge/ani_thread_state_and_scopes.md` |
| `ani.h` / public API / ABI / handle typedef / `ani_status` / API-table ordering / external interface sync | `docs/knowledge/ani_public_api_compatibility.md` |

## Constraints And Boundaries

### Core Constraints

- Public `ani.h` declarations express the stable ANI contract; do not change signatures, typedefs, enum values, struct layout, or API-table ordering without treating it as a public compatibility change.
- Public interface changes in `ani.h` must be synchronized with the OpenHarmony `interface_sdk_c` repository file `ani/ani.h`; read `docs/knowledge/ani_public_api_compatibility.md` before planning such changes.
- Internal ETS objects and verifier wrappers must not leak into the public API.
- Release ANI and VerifyANI must stay behaviorally aligned for shared APIs; detailed verifier handle, checker, and reporting rules belong to `verify/AGENTS.md`.
- `PandaAniEnv::FromAniEnv` must continue to unwrap `VEnv` before accessing the owner `PandaAniEnv`.
- Reference operations must go through `EtsReferenceStorage` via `ManagedCodeAccessor`/`ScopedManagedCodeFix`; do not reinterpret handles and bypass storage ownership checks.
- Native bridge changes must preserve architecture-specific argument layout and managed/native transition semantics.
- Do not bypass pending-exception checks, type checks, reference-kind checks, or scope checks to make tests pass.

### Ask Before

- Changing public ANI semantics, ABI layout, status code meaning, or handle identity rules.
- Adding production dependencies or moving ANI code across module boundaries.
- Deleting compatibility behavior, workaround paths, bridge entry points, or verifier coverage.
- Running commands that affect connected devices or external OpenHarmony interface repositories.

## Verification

Run commands from the project's build output directory (e.g., `runtime_core/static_core/out/`). If the directory is missing or stale, configure/build it with the project build workflow before running tests. If validation cannot be run, state the reason in the final response.

Minimum checks:

- Full ANI tests: `ninja ani_tests`
- Focused ANI test: `ninja <test_name>_gtests`

