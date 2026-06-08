# Compiler Workflows Knowledge

This file records high-level routing, purpose, boundaries, and verification expectations for ArkTS-Sta compiler workflows. It covers JIT, OSR, AOT, LLVM AOT, and paoc validation modes.

## Purpose

The compiler turns Ark bytecode into executable code under different timing and deployment constraints. The workflows share compiler concepts, but they serve different goals and have different evidence requirements.

| Workflow | Purpose | Boundary |
|---|---|---|
| JIT | Compile hot methods during execution to improve steady-state performance | Must preserve runtime responsiveness, profiling meaning, and deoptimization safety |
| OSR | Enter compiled code from a currently running loop | Must preserve live state, stack visibility, and interpreter-to-compiled transition semantics |
| AOT | Compile before execution and let runtime reuse the compiled result | Must match the application context used at runtime and remain diagnosable when it cannot be used |
| LLVM AOT | Use the LLVM backend for eligible AOT compilation | Must distinguish backend eligibility/fallback from semantic correctness |
| paoc validation modes | Exercise compiler graph building and codegen without producing a deployable AOT result | Must not be treated as proof that runtime AOT loading works |

## Workflow Selection Model

| Workflow | When it is selected | Main output | Success evidence |
|---|---|---|---|
| JIT | Runtime sees a hot method and policy allows JIT | Installed compiled entry in the running VM | Method is selected, compiled, installed, and executed as compiled code |
| OSR | Runtime sees a hot loop in a currently executing frame and OSR policy allows transition | Loop-entry compiled code plus live-state mapping | OSR entry happens and values survive the interpreter-to-compiled transition |
| AOT | `ark_aot` compiles application inputs before execution | AOT product consumable by runtime | Product is generated, loadable, matched, and used during execution |
| LLVM AOT | `ark_aot --paoc-mode=llvm` routes eligible graphs to LLVM backend | LLVM-backed AOT product or controlled fallback | LLVM path participation or explicit rejection/fallback is visible |
| paoc `jit`/`osr` | Offline compiler validation mode is requested | Compiler pipeline evidence, not a deployable product | Graph/codegen behavior is validated without claiming runtime product loading |

## Shared Boundaries

- Language behavior: compiler workflow changes must not change user-visible language behavior.
- Runtime contract: compiled code must remain compatible with runtime entry, stack walking, exceptions, GC, and deoptimization expectations.
- Metadata contract: `CodeInfo`, stack maps, inline info, roots masks, bridge frames, and deopt metadata are correctness data.
- Pointer contract: compiled/runtime boundaries must prove pointer lifetime, non-null invariants, and GC visibility for object/reference values crossing calls, safepoints, deopt, OSR, bridges, or generated fastpaths.
- Mode differences: a fix for one mode must be checked against other modes that share pipeline, analysis, optimization, or codegen paths.
- Profile data: profile-guided decisions are valid only when the profile represents the current code and runtime context.
- Runtime evidence: offline `paoc` validation, dumps, or backend selection proof are not runtime proof for JIT install, OSR transition, or AOT loading.
- Diagnostics: dumps, logs, and failure messages are part of the debugging contract and should not be weakened to make a test pass.

## Mode Difference Checklist

- Runtime timing: JIT and OSR run while the VM is executing; AOT and LLVM AOT run before execution.
- Entry shape: OSR enters a loop with live interpreter state; normal JIT and AOT enter at method entry.
- Class context: AOT and AOT PGO depend on stable file/class context; JIT can observe runtime state directly.
- Backend: LLVM AOT may reject or fallback for backend eligibility reasons that normal AOT does not have.
- Evidence layer: compiler success, product generation, product load, and compiled execution are different claims.

## Code and Test Anchors

| Workflow | Owning surfaces |
|---|---|
| JIT | `static_core/compiler/optimizer/`, `static_core/runtime/compiler*`, `static_core/tests/checked/` |
| OSR | `static_core/compiler/optimizer/`, `static_core/runtime/osr.*`, `static_core/runtime/arch/*/osr_*`, `static_core/tests/checked/osr*.pa` |
| AOT | `static_core/compiler/aot/`, `static_core/compiler/tools/paoc/`, `static_core/compiler/tools/aotdump/`, `static_core/tests/checked/aot*.pa` |
| LLVM AOT | `static_core/compiler/aot/aot_builder/`, `static_core/compiler/tools/paoc/paoc_llvm.*`, `static_core/libllvmbackend/`, `static_core/tests/checked/llvm*.pa` |
| Profile-guided AOT | `static_core/runtime/jit/`, `static_core/compiler/tools/aptool/`, AOT PGO runner workflows |

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| Compiler flags, dumps, mode overview | `static_core/compiler/docs/compiler_doc.md` |
| Compiler-owned doc index | `static_core/compiler/docs/README.md` |
| shared pipeline / generated sources / pass debugging | `static_core/compiler/docs/knowledge/compiler-pipeline-debugging.md` |
| IR format and bytecode-to-IR basics | `static_core/docs/ir_format.md`, `static_core/compiler/docs/ir_builder.md` |
| Performance and benchmark investigation | `static_core/compiler/docs/performance_workflows.md` |

## Common Misroutes

| Symptom | Do not assume | Check first |
|---|---|---|
| A method is not using compiled code | Compiler failed | Whether the selected workflow, input context, filters, or runtime policy allow compiled-code use |
| AOT run falls back | Fallback is success | Whether the target method was actually covered and loaded |
| JIT-only failure | Codegen bug only | Whether profiling, runtime installation, deoptimization, or GC visibility differs from AOT |
| OSR-only failure | Loop optimization bug only | Whether live state and transition semantics are preserved |
| LLVM AOT differs from AOT | LLVM is wrong | Whether eligibility, fallback policy, or backend-specific limits explain the difference |

## Modification Checklist

Before editing, confirm:

- The primary target is named explicitly: JIT, OSR, AOT, LLVM AOT, or paoc validation.
- Other affected workflows are listed with verification or an explicit non-impact reason when shared compiler behavior changes.
- Runtime guides and relevant runtime tests/logs are included when runtime state affects the result.
- Matching and mismatching profile scenarios are considered when profile data is involved.
- Fallback and mode-selection semantics are named explicitly when a change can alter which workflow actually runs.
- JIT, OSR, AOT/AOT-PGO, and LLVM AOT claims use workflow-specific proof rather than fallback or offline evidence.
- Direct evidence from dumps, logs, or targeted tests supports any compiled-code-use claim.

## Ask Before

- changing trigger policy, hotness defaults, async-vs-sync defaults, or fallback semantics that can move work between JIT, OSR, AOT, LLVM AOT, and interpreted execution;
- weakening AOT or AOT PGO matching/context requirements, or changing what counts as acceptable runtime reuse of a compiled product;
- changing metadata ownership or requirements for stack maps, `CodeInfo`, inline info, deopt, GC roots, OSR, or bridge frames;
- using offline `paoc`/backend evidence as the only justification for runtime workflow behavior changes.

## Verification

- Compiler still builds: relevant compiler target builds successfully.
- Mode-specific behavior holds: targeted JIT, OSR, AOT, or LLVM AOT test reproduces the intended behavior.
- Shared behavior is stable: compiler unit/codegen tests or checked tests cover shared paths.
- Runtime boundary is safe: runtime-facing tests or logs cover installation, transition, stack walking, exception, or deopt as applicable.
- Diagnostics remain useful: failure paths still expose enough context to identify mode, input, and reason.
