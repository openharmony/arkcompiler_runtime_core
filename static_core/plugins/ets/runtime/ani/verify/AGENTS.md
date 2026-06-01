# VerifyANI Agent Guide

## Code Map

This AGENTS.md applies to `runtime_core/static_core/plugins/ets/runtime/ani/verify/`. It adds VerifyANI-specific rules on top of `../AGENTS.md`.

VerifyANI wraps the release ANI API with checked handles, per-env frame state, global verifier state, and argument validation. The most important boundary is: VerifyANI may validate and report invalid native usage, but it must preserve the release ANI API surface and must call through to the release interaction/VM APIs for real runtime behavior.

Key areas:

- `verify_ani_interaction_api.cpp/h`: verified interaction API table and wrappers around release interaction API.
- `verify_ani_vm_api.cpp/h`: verified VM API table and VM/thread wrappers.
- `verify_ani_checker.cpp/h`: argument model, checker dispatch, and validation helpers.
- `verify_ani_cast_api.h`: casts between verified wrappers and release ANI handles.
- `ani_verifier.cpp/h`: VM-wide verified global refs, weak refs, method/field/resolver state, report hooks.
- `env_ani_verifier.cpp/h`: per-env native frames, local scopes, escape scopes, local verified refs.
- `types/`: `VEnv`, `VRef`, `VWRef`, `VMethod`, `VField`, `VResolver`, and internal reference wrappers.
- `docs/knowledge/`: VerifyANI-specific agent knowledge.
- `../../../tests/ani/tests/verifyani/`: VerifyANI test sources and generated-test templates.

## Knowledge Routing

Use the table below as keyword-based routing before planning or editing.

| Keywords | Read |
| --- | --- |
| VerifyANI mode / `VEnv` / `VRef` / verified state / `frames_` / `NATIVE_FRAME` / `LOCAL_SCOPE` / `ESCAPE_LOCAL_SCOPE` | `docs/knowledge/verifyani_architecture.md` |
| checker / validation / `VERIFY_ANI_ARGS` / `VERIFY_ANI_ABORT_IF_ERROR` / `ANI_VERIFICATION_MAP` / `ANI_ARG_TYPES_MAP` | `docs/knowledge/verifyani_checker_rules.md` |
| reporting / abort hook / error hook / invalid-usage policy / `IsWorkaroundNoCrashIfInvalidUsage` | `docs/knowledge/verifyani_error_reporting.md` |
| local verified ref / global verified ref / weak verified ref / `AddLocalVerifiedRef` / `DeleteLocalVerifiedRef` / scope validity | `docs/knowledge/verifyani_architecture.md`, `../docs/knowledge/ani_reference_lifecycle.md` |
| thread attach / thread detach / native frame push / native frame pop | `../docs/knowledge/ani_thread_state_and_scopes.md`, `docs/knowledge/verifyani_architecture.md` |
| API parity with release ANI / public API / API-table layout | `../docs/knowledge/ani_public_api_compatibility.md`, `docs/knowledge/verifyani_architecture.md` |

## Constraints And Boundaries

### Core Constraints

- VerifyANI APIs must remain one-to-one with the release ANI APIs they wrap.
- Verified wrappers must unwrap to release ANI handles before calling release interaction APIs.
- Local verified refs must be created and destroyed through `EnvANIVerifier`; global verified refs, weak refs, methods, fields, and resolvers must be managed through `ANIVerifier`.
- Do not store release local refs in VM-wide verifier maps.
- Keep `IsWorkaroundNoCrashIfInvalidUsage` private.

### Ask Before

- Changing VerifyANI behavior so it no longer matches the release ANI API on successful calls.
- Changing invalid-usage reporting from fatal to non-fatal, from non-fatal to fatal, or changing abort/error hook semantics.
- Exposing `IsWorkaroundNoCrashIfInvalidUsage`, adding new public verifier options, or making workaround policy visible outside `ANIVerifier`.
- Moving verified handle ownership between `EnvANIVerifier` and `ANIVerifier`, or changing when local/global/weak verified refs are cleaned up.
- Deleting verifier checks, generated-test templates, negative tests, or compatibility workarounds.

## Verification

Run commands from the project's build output directory (e.g., `runtime_core/static_core/out/`). If the directory is missing or stale, configure/build it with the project build workflow before running tests.

Minimum checks:

- Full VerifyANI tests: `ninja ani_verify_tests`
- Focused VerifyANI test: `ninja <ani_verify_test_name>_gtests`

For generated VerifyANI tests, confirm the generated target name in the local `CMakeLists.txt`. If a checker change also affects release ANI semantics, run the matching release `ani_test_*_gtests` target. If validation cannot be run, state the reason in the final response.
