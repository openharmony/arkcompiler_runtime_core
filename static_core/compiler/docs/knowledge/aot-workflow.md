# AOT Workflow Knowledge

This file covers ArkTS-Sta AOT workflow: offline compilation, deployable compiled output, runtime matching, loading, and execution. It avoids file-format implementation details and focuses on contract boundaries.

## Purpose

AOT compiles before execution so runtime can reuse compiled code. It reduces runtime compilation cost, but it must preserve bytecode behavior and remain tied to the application context it was built for.

## Lifecycle And Contract

| Stage / contract | What to prove | Risk |
|---|---|---|
| Input context | Intended panda files, location/path context, boot files, options, and optional profile were used | Mismatch can make output unusable or silently unoptimized |
| Offline compilation | Target methods were compiled or rejected with reason | Shared compiler changes may affect JIT/OSR too |
| Product inspection | aotdump/logs show expected coverage and metadata | Contract changes affect diagnostics and compatibility |
| Runtime matching/loading | Runtime loader/class linker accepts the product for the current application context | Fallback can hide AOT failure |
| Execution | Runtime proves compiled AOT code was used, not only available | Runtime boundary must remain compatible |

## Code and Test Anchors

- AOT product model starts in `static_core/compiler/aot/`.
- Offline compiler entry starts in `static_core/compiler/tools/paoc/`.
- Product diagnostics start in `static_core/compiler/tools/aotdump/`.
- Profile-guided AOT crosses `static_core/runtime/jit/` and `static_core/compiler/tools/aptool/`.
- AOT validation should include `static_core/tests/checked/aot*.pa` or AOT runner workflows.

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| AOT file format, headers, GOT, runtime calls | `static_core/docs/aot.md` |
| AOT on device and boot AOT lookup | `static_core/docs/aot_on_device.md` |
| `ark_aot` / paoc modes and options | `static_core/compiler/docs/paoc.md`, `static_core/compiler/tools/paoc/paoc.yaml` |
| AOT class hierarchy analysis | `static_core/compiler/docs/aot_cha.md` |
| AOT PGO workflow | `static_core/compiler/docs/aot_pgo.md` |
| AOT string resolution | `static_core/compiler/docs/aot_resolve_string.md` |
| AOT PLT and call indirection | `static_core/compiler/docs/plt.md` |

## Boundaries

- Do not treat AOT output as a generic cache independent of input context.
- Do not claim AOT success from `ark_aot` success alone; runtime load and use must be proven.
- Do not weaken matching rules to make an output load if the application context is inconsistent.
- AOT/AOT-PGO changes must prove class context, panda file set, locations, checksums, and profile context match or reject clearly.
- Do not remove dump/log evidence for load, coverage, or failure reasons.

## Common Misroutes

- If output builds but runtime ignores it, check path/location, panda file set, load options, and fallback first.
- If a method is not covered, check filters, external method rules, unsupported constructs, and dump evidence.
- If a failure is AOT-only, check runtime loading, class linking, entrypoints, and stack metadata.
- If profile-guided AOT changes nothing, check profile match, profile content, and option shape.

## Modification Checklist

Before editing, confirm:

- Compatibility and diagnostics are described when the AOT input/output contract changes.
- Load and execution evidence is planned when runtime use is part of the claim.
- Match and mismatch behavior are covered when a profile is involved.
- Runtime boundary safety covers pointer lifetime, non-null invariants, GC visibility, `CodeInfo`, stack maps, roots masks, and deopt metadata when touched.
- JIT, OSR, and LLVM AOT impact is considered when shared compiler code changes.

## Verification

- Output is generated: `ark_aot` or runner succeeds.
- Output is inspectable: dump/log identifies relevant content.
- Output is used: runtime execution proves loaded compiled code is used.
- Fallback is understood: fallback or rejection reason is explicit.
- Behavior is equivalent: non-AOT and AOT outputs/exceptions match.
