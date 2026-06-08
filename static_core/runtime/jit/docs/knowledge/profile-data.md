# Runtime Profile Data Knowledge

This file covers runtime profile data and `.ap` profile IO used by JIT-related decisions and AOT PGO. It does not cover JIT queues, OSR entry, or compiled code installation.

## Purpose

Profile data records observed runtime behavior so compiler workflows can make better decisions. It is only useful when the recorded context matches the code and runtime context that consumes it.

## Lifecycle

| Stage | Owner | Risk |
|---|---|---|
| Collection | Interpreter/runtime profiling | Missing threshold or disabled profiling produces empty data |
| Save | Runtime profile saver | File existence alone does not prove useful content |
| Inspect | `ark_aptool` or equivalent tooling | Unreadable profiles cannot support optimization claims |
| Load | Runtime/paoc profile loader | Context mismatch must be rejected, not ignored silently |
| Consume | Compiler optimizations through runtime interface | Loaded profile must be shown to affect the intended decision |

## Model

| Data | Purpose | Risk |
|---|---|---|
| Inline cache | Receiver/type observations for calls | Stale context can misguide inlining |
| Branch data | Observed branch direction | Missing data can skew layout decisions |
| Throw data | Observed exceptional behavior | Rare paths can be underrepresented |
| `.ap` file | Persisted profile data for offline use | Format/context mismatch must be rejected |
| Class context | Validates profile against loaded files/classes | Weak matching can attach wrong data |

## Context Keys

| Context | Why it matters |
|---|---|
| Panda file set | Profiles are tied to the code that produced them |
| Class context | Prevents attaching observations to the wrong loaded classes |
| Method identity | Optimizations must read data for the intended method |
| Runtime/profile options | Thresholds and branch collection affect what data exists |
| `force` use | Can help diagnostics but must not hide semantic mismatch |

## AOT PGO Use Rules

| Rule | Reason |
|---|---|
| `--paoc-use-profile:path=<profile.ap>[,force]` must name the profile path | The profile is not implied by runtime save options |
| Mirror `--paoc-panda-files` into `--panda-files` when applying a profile | The runtime context must load the same application files before profile attachment |
| A profile file existing is not enough | It must contain useful data and pass context checks |
| AOT PGO does not remove normal AOT eligibility limits | External methods, unavailable classes, and load-context limits still apply |
| Inline-cache guided AOT inlining still follows AOT external-inlining rules | Profile data is guidance, not permission to bypass AOT constraints |

## Runtime Options And Integration

Saving a useful `.ap` profile requires runtime profile collection, profile saving, and an output path to be configured. Use `static_core/runtime/options.yaml` as the source of truth for exact runtime option names.

Commonly relevant option areas include:

- profile collection enablement;
- profile saver enablement;
- incremental profile saver enablement for background saves;
- profile output path;
- compiler profiling threshold;
- branch profile collection when branch detail is needed.

Runtime profiling starts once methods cross the profiling threshold. Profiles can be saved on shutdown or by incremental profile-saver flows.

## Owners And Anchors

- `static_core/runtime/options.yaml` owns profiling and hotness thresholds.
- `static_core/runtime/jit/profile_saver_worker.*` owns incremental/background profile saving; it is not a JIT compiler worker.
- `static_core/runtime/jit/profiling_data.*`, `profiling_loader.*`, `profiling_saver.*`, and `libprofile/` own runtime profile data, IO, and persisted profile model.
- `static_core/compiler/tools/paoc/paoc.cpp` owns AOT PGO loading and use.
- `static_core/compiler/tools/aptool/` owns human-readable profile inspection.
- `static_core/compiler/tests/profiling_*`, `static_core/compiler/tests/aptool_dump_test.cpp`, `static_core/runtime/tests/profile_merger_test.cpp`, and AOT PGO runner workflows cover compiler/runtime validation.
- `static_core/compiler/docs/performance_workflows.md` covers perf workflows that may consume profiles.

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| Bytecode profiling and `.ap` overview | `static_core/docs/bytecode_profiling.md` |
| `ark_aptool` usage | `static_core/docs/aptool.md` |
| AOT PGO consumption | `static_core/compiler/docs/aot_pgo.md` |
| paoc profile option source | `static_core/compiler/docs/paoc.md`, `static_core/compiler/tools/paoc/paoc.yaml` |

## Boundaries

- Do not use profile data when class context or panda file context does not match.
- AOT-PGO changes must prove class context, panda file set, locations, checksums, and profile context match or reject clearly.
- Do not confuse profile saving with JIT code installation.
- Do not make profile-guided optimization claims without proving the profile was loaded and consumed.
- Do not remove mismatch diagnostics; failed profile attachment must remain explainable.
- Do not loosen profile matching, rejection behavior, or aptool visibility to increase apparent profile reuse.

## Common Misroutes

- If AOT PGO is ignored, check the profile path option, application files loaded for matching, and class context first.
- If a profile file exists but has no effect, check content, method names, profile thresholds, and consumer options.
- If JIT behavior differs, check runtime hotness and profile collection before assuming compiler pass logic is wrong.
- If profile load fails, check context mismatch, format compatibility, and diagnostics.

## Modification Checklist

Before editing, confirm:

- The owner path is profile data rather than JIT orchestration.
- Persisted format changes include compatibility and rejection behavior.
- AOT PGO changes test matching and mismatching contexts.
- Optimization claims are backed by visible profile use in logs, dumps, or tests.
- Pointer lifetime and non-null invariants are explicit for profile/class-context data touched by the change.

## Verification

- Profile is saved: `.ap` exists and contains expected data.
- Profile is inspectable: aptool or equivalent output is readable.
- Profile is accepted: matching context loads successfully.
- Profile is rejected safely: mismatching context gives a clear failure.
