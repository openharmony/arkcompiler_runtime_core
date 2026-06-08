# Compiled Runtime Boundary Knowledge

This file covers runtime boundaries shared by JIT, OSR, AOT, and LLVM AOT: entrypoints, bridges, stack walking, deoptimization, class linking, and compiled-code installation.

## Purpose

Compiled code must cooperate with runtime services. The runtime boundary preserves execution state, class/linking state, exceptions, GC visibility, and diagnostics when execution crosses between interpreted, compiled, and runtime code.

## Boundary Map

| Boundary | Purpose | Risk |
|---|---|---|
| Entrypoints | Compiler-visible runtime service calls | Signature/semantic drift breaks compiled code |
| Bridges | Interpreter/compiled/runtime transitions | ABI mistakes cause crashes or corrupted state |
| Stack walker | Runtime view of compiled frames | GC, exceptions, deopt, and debugging depend on it |
| Deoptimization | Return from compiled code to interpreter state | Missing state corrupts later execution |
| Class linker | Connects methods/classes to runtime metadata and AOT code | Bad matching can install wrong code or fallback silently |

## Code and Test Anchors

Owners start in runtime compilation/OSR code (`static_core/runtime/compiler*`, `static_core/runtime/compiler_*`, `static_core/runtime/osr.*`, `static_core/runtime/arch/*/osr_*`), compiler runtime interfaces (`static_core/runtime/include/compiler_interface*.h`), and compiled metadata/product code (`static_core/compiler/aot/`, `static_core/compiler/code_info/`). Validate with `static_core/runtime/tests/` or checked workflow tests as applicable.

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| Compiled/runtime interaction overview | `static_core/docs/runtime-compiled_code-interaction.md` |
| Code metadata and stack maps | `static_core/docs/code_metainfo.md` |
| Deoptimization behavior | `static_core/docs/deoptimization.md` |
| Bridges and compiler-side boundary notes | `static_core/compiler/docs/bridges.md`, `static_core/compiler/docs/codegen_doc.md` |

## Boundaries

- Do not update compiler call sites without validating runtime entrypoint semantics.
- Do not update bridge/stack metadata without runtime and compiler-side evidence.
- Stack maps, `CodeInfo`, GC metadata, and deopt metadata are required boundary contract, not optional bookkeeping.
- Inline info, roots masks, bridge frames, and deopt metadata are correctness data, not optional diagnostics.
- Do not rely on unchecked raw pointers at compiled/runtime boundaries; prove ownership/lifetime/non-null invariants or keep explicit checks.
- Do not bypass class initialization, exception, GC, or stack checks to make compiled execution pass.
- Do not weaken class-linker, AOT, or AOT-PGO matching/context checks to increase compiled-code use.
- Do not treat silent fallback as successful compiled-code execution.

## Ask Before

- changing ABI-visible entrypoints, bridge conventions, stack-map or `CodeInfo` layout/ownership, or deopt metadata requirements;
- changing roots masks, inline info, bridge-frame, OSR, or stack-map requirements;
- changing class-linker matching, class-context assumptions, or AOT/AOT-PGO reuse criteria;
- changing fallback semantics in ways that can hide boundary breakage.

## Common Misroutes

- If a crash happens after code install, check bridges, entrypoints, stack maps, and calling convention first.
- If an exception path differs, check deopt metadata, stack walker behavior, and runtime call semantics.
- If an AOT method is not used, check class linker matching, load diagnostics, and fallback path.
- If a crash is GC-sensitive, check safepoints, roots, stack maps, and object visibility.

## Modification Checklist

Before editing, confirm:

- The changed boundary is named: entrypoint, bridge, stack, deopt, or linker.
- If no null check exists, ownership, lifetime, or invariant evidence proves the value is non-null.
- Object/reference values live across runtime calls, safepoints, deopt, OSR, bridges, or generated fastpaths stay visible through `SaveState`, roots, stack maps, or handles.
- Compiler-side validation is included when compiler metadata is affected.
- Exception, GC, and class-init implications are checked when runtime state is affected.
- Success and fallback are distinguishable when fallback is possible.

## Verification

- Boundary call works: runtime/compiler execution test passes.
- Stack is visible: stack walker, deopt, or exception scenario is covered when relevant.
- GC safety holds: GC-sensitive path is covered when object state is involved.
- Diagnostics remain clear: failure or fallback reason is visible.
