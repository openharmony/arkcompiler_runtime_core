# irtoc Generated Code Knowledge

This file covers irtoc generated-code work: `.irt` sources, DSL validation, generated interpreter handlers, and generated helper code. FastPath and NativePlus boundary work is covered by `irtoc-fastpath.md`.

## Purpose

irtoc generated code keeps selected interpreter and helper paths consistent with compiler IR while avoiding handwritten duplicated low-level code. It should preserve runtime semantics and remain small enough for generated-code validation to be meaningful.

## Model

| Area | Purpose | Risk |
|---|---|---|
| `.irt` source | Declares generated handler/helper behavior | Broad semantics can become hard to validate |
| DSL validation | Guards shape, size, and generated-code constraints | Weakening validation hides long-term risk |
| Generated interpreter | Executes bytecode handlers through generated code | Must match interpreter/runtime semantics |
| Backend lowering | Converts irtoc model through compiler backend | Backend assumptions can affect target behavior |

## Source Of Truth

| Item | Owner / route |
|---|---|
| Ruby DSL frontend | `static_core/irtoc/lang/` |
| DSL validation | `static_core/irtoc/lang/validation.rb` |
| Core `.irt` behavior | `static_core/irtoc/scripts/` |
| ETS `.irt` behavior | `static_core/plugins/ets/irtoc_scripts/` |
| Compiler-side lowering | `static_core/irtoc/backend/` |
| Generated build outputs | build directory outputs such as `irtoc_code.cpp`, optional LLVM irtoc code, and disassembly |

## Generation Targets

The irtoc frontend is a Ruby DSL rather than a separate parser. Current generation targets include:

- `ir-constructor` for constructing compiler IR from irtoc source.
- `ir-builder` for compiler-side generated builders.
- `ir-inline` for inline generation paths.

Runtime builds generate outputs such as `irtoc_code.cpp` and, when enabled, optional LLVM irtoc output before lowering through the normal compiler backend. When a generated output is wrong, inspect both the `.irt` source and the generated build artifact.

## Script Groups

The `.irt` sources are not interchangeable. Start from the smallest owner that matches the behavior:

- `scripts/interpreter.irt` owns interpreter handlers and JIT/OSR trigger points.
- `scripts/allocation.irt`, `gc.irt`, `monitors.irt`, and `check_cast.irt` cover common fast paths and slow-path glue.
- `scripts/string_helpers.irt`, `strings.irt`, `string_builder.irt`, and `arrays.irt` cover helpers that compiler intrinsics often reach through fast paths.
- `scripts/tests.irt` is for irtoc-focused test handlers.
- `static_core/plugins/ets/irtoc_scripts/` owns ETS-specific interpreter and fastpath additions wired through plugin options.

## Code and Test Anchors

Owners are `.irt` sources (`static_core/irtoc/scripts/`, `static_core/plugins/ets/irtoc_scripts/`), DSL validation (`static_core/irtoc/lang/`), and lowering/backend code (`static_core/irtoc/backend/`). Validate with `irtoc_unit_tests`, `irtoc_compiler_unit_tests`, or `irtoc-interpreter-tests` as applicable.

## Documentation Routing

| Scenario / Keywords | Read first |
|---|---|
| irtoc pipeline and generation targets | `static_core/docs/irtoc.md` |
| interpreter intrinsic generation | `static_core/docs/irtoc-intrinsics-for-interpreter.md` |
| interpreter execution context | `static_core/docs/interpreter-knowledge.md`, `static_core/docs/design-of-interpreter.md` |

## Test Commands

```sh
ninja irtoc_unit_tests
ninja irtoc_compiler_unit_tests
ninja irtoc-interpreter-tests
```

Use `static_core/tests/AGENTS.md` for interpreter/runner-based validation beyond these direct targets.

## Debugging Hints

- For interpreter bugs, correlate `scripts/interpreter.irt` with `static_core/runtime/interpreter/`.
- For OSR/JIT trigger bugs, inspect `scripts/interpreter.irt`, `static_core/runtime/interpreter/instruction_handler_base.h`, and `static_core/runtime/osr.cpp`.
- For generated-output bugs, inspect the build directory outputs such as `irtoc_code.cpp`, optional `irtoc_code_llvm.cpp`, and `disasm.txt`.

## Boundaries

- Do not use irtoc generated code as a place for broad language behavior.
- Do not bypass validation unless the size/shape and target impact are explicitly justified.
- Do not weaken irtoc validation, matching, rejection, fallback diagnostics, or fastpath ABI.
- Do not change generated interpreter behavior without comparing against the runtime/interpreter owner.
- Do not assume generated-code success proves fastpath/nativeplus ABI correctness.
- Do not bypass stack walking, exception, GC, class-init, deopt, OSR, or generated fastpath checks.

## Common Misroutes

- If a generated handler differs from the interpreter, check the runtime semantic owner and `.irt` source shape.
- If validation fails, check helper complexity plus spill and code-size pressure.
- If a failure is target-only, check backend lowering and target constraints.
- If an OSR/JIT trigger differs, check interpreter trigger semantics and runtime transition ownership.

## Modification Checklist

Before editing, confirm:

- The task is generated interpreter/helper work, not actually a fastpath or NativePlus ABI issue.
- The generated logic is small and local; broad behavior remains in runtime/compiler owners.
- Validation changes include explicit evidence.
- Pointer lifetime, non-null invariants, and `SaveState`/roots/stack-map visibility are proven for object/reference values crossing runtime boundaries.
- Runtime behavior changes are verified with runtime/interpreter execution.

## Verification

- DSL/source is valid: `irtoc_unit_tests`.
- Backend lowering is valid: `irtoc_compiler_unit_tests`.
- Interpreter behavior is correct: `irtoc-interpreter-tests` or focused runtime execution.
- Workflow interaction is safe: checked/runtime test when JIT, OSR, or compiler interaction is involved.
