# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: irtoc
- **purpose**: Generates optimized interpreter handlers and fastpath/nativeplus entrypoints for the runtime; also provides intrinsic-specific code generation support
- **primary language**: Ruby, C++

## About irtoc

**irtoc** is the IR-to-Code toolchain used to generate:

- interpreter handlers
- fastpath entrypoints used by compiler-generated `CallFastPath`
- nativeplus/boundary entrypoints
- helper intrinsics used by the runtime/compiler boundary

## Current Implementation Notes

- the front-end is a Ruby DSL; it does not maintain a separate custom parser
- the DSL is backed by `compiler/optimizer/ir/instructions.yaml`
- current generation targets include `ir-constructor`, `ir-builder`, and `ir-inline`
- runtime builds generate `irtoc_code.cpp` and optionally `irtoc_code_llvm.cpp`, then lower them through the normal
  compiler backend into final object files
- compiler-side generation also reuses irtoc sources through `irtoc_generate(... IR_API ir-builder|ir-inline ...)`

## Modes And ABI Variants

- `FastPath` is the normal low-overhead mode for compiler/runtime entrypoints
- `NativePlus` is the native-ABI variant used when interpreter/runtime bridges call irtoc code directly
- `FastPathPlus` is the convenience macro-mode that emits both `FastPath` and `NativePlus` variants
- arm32 still has irtoc codegen branches and register maps, but the runtime keeps interpreter selection on the C++
  path there; do not collapse those two facts into "arm32 unsupported"

## Directory Structure

Main File Directories

```
irtoc/
├── backend/                    # C++ backend for lowering IRtoC output to compiler IR/code
│   ├── compilation.*          # compilation driver
│   ├── function.*             # IRtoC function model
│   ├── options.yaml           # backend options
│   └── compiler/
│       ├── codegen_interpreter.h   # interpreter codegen
│       ├── codegen_fastpath.*      # fastpath codegen
│       ├── codegen_nativeplus.*    # nativeplus codegen
│       └── constants.h
│
├── lang/                       # Ruby DSL and validation logic
│   └── validation.rb          # validates generated methods against spill/code-size limits
│
├── scripts/                    # IR source files (.irt)
│   ├── common.irt             # Common definitions and utilities
│   ├── interpreter.irt         # Core interpreter handlers
│   ├── allocation.irt         # Object allocation handlers
│   ├── arrays.irt             # Array operations
│   ├── strings.irt            # String operations
│   ├── string_builder.irt     # StringBuilder operations
│   ├── string_helpers.irt     # String helper functions
│   ├── gc.irt                 # Garbage collection helpers
│   ├── memcopy.irt            # Memory copy operations
│   ├── array_helpers.irt      # Array helper functions
│   ├── resolvers.irt          # Symbol resolution
│   ├── monitors.irt           # Synchronization/monitors
│   ├── check_cast.irt         # Type checking and casting
│   └── tests.irt              # Test handlers
│
├── templates/                  # Build templates
│
├── intrinsics.yaml             # Intrinsic function definitions
├── irtoc.gni                   # GN build configuration
├── BUILD.gn                    # GN build file
├── CMakeLists.txt              # CMake build configuration
└── README.md                   # Basic documentation
```

## Generated Targets and Build

- core runtime build consumes generated `irtoc_interpreter` and `irtoc_fastpath` objects from `runtime/CMakeLists.txt`
- ETS runtime additionally builds and links `irtoc_ets_fastpath` from `plugins/ets/runtime/CMakeLists.txt`
- `irtoc_unit_tests` runs Ruby frontend tests (`lang/tests/*`)
- `irtoc_compiler_unit_tests` covers backend/compiler-side logic
- `irtoc-interpreter-tests` in `tests/` validates generated interpreter behavior against PA tests

## IR Language (.irt files)

The IR language is a Ruby-based DSL for describing bytecode handler logic.

Key script groups:

- `scripts/interpreter.irt` - interpreter handlers and JIT/OSR trigger points
- `scripts/allocation.irt`, `gc.irt`, `monitors.irt`, `check_cast.irt` - common fast paths and slow-path glue
- `scripts/string_helpers.irt`, `strings.irt`, `string_builder.irt`, `arrays.irt` - string/array helpers that compiler intrinsics often reach through fast paths
- `scripts/tests.irt` - irtoc-focused test handlers
- `plugins/ets/irtoc_scripts/` - ETS-specific interpreter and fastpath additions wired through plugin options

## Fast Paths and Validation

- `backend/compiler/codegen_fastpath.*` implements fastpath code generation used by runtime/compiler entrypoints
- fastpath code has stricter constraints than normal compiled code; validation in `lang/validation.rb` checks limits such as maximum spill count and code size
- `runtime/CMakeLists.txt` wires core `irtoc_fastpath` into the runtime and may skip validation for some LLVM fastpath configurations; ETS builds additionally wire `irtoc_ets_fastpath` from `plugins/ets/runtime/CMakeLists.txt`
- if a compiler intrinsic calls `CallFastPath`, the generated entrypoint is typically in `irtoc/` or runtime entrypoints, not in `compiler/` alone
- current fastpath code generally relies on explicit `SLOW_PATH_ENTRY` or `TAIL_CALL` transfers instead of arbitrary
  nested calls
- `LiveIn`, `LiveOut`, `Label`, `Goto`, `Else`, `macro`, `scoped_macro`, `word`, and `ref_uint` are part of normal
  current irtoc work, not historical one-offs

## Choosing irtoc

- `irtoc` is primarily for short critical paths where reducing `FastPath` or boundary-call overhead matters more than the optimizer flexibility lost by the special `irtoc` compilation mode
- Evaluate each candidate against the competing implementation choices: inline managed code, a regular managed call, a C++ runtime or intrinsic implementation, and an `irtoc` fastpath
- The most common good fits are extremely hot trivial constructors or allocation helpers, and tiny low-level sequences where managed source or C++ cannot be lowered into the desired machine instructions
- In the second case, treat `irtoc` as a constrained assembler wrapper and keep the low-level sequence as small as possible
- Keep `irtoc` bodies small; long, branch-heavy, or general-purpose logic usually belongs in managed code or C++
- If a larger `irtoc` implementation materially outperforms the high-level or C++ version, investigate algorithmic issues or missing optimizer or codegen support before making `irtoc` the long-term solution

## Review Checklist

- Why is inlining or a normal runtime or native call not sufficient here?
- Is the code path both hot enough and small enough for `FastPath` overhead reduction to matter?
- Is the expected win tied to a specific target, ABI, or instruction-selection gap? Validate on the relevant architecture and build mode
- Did you keep the semantic complexity in managed or C++ code where possible and limit `irtoc` to the performance-critical core?
- Did you add or update coverage in `irtoc_unit_tests`, `irtoc_compiler_unit_tests`, interpreter tests, and the compiler or runtime execution layer?

## Debugging Hints

- For interpreter bugs, correlate `scripts/interpreter.irt` with `runtime/interpreter/`
- For fastpath bugs, correlate `codegen_fastpath.*` with `compiler/optimizer/code_generator/codegen-inl.h`, `runtime/compiler.cpp`, and runtime entrypoints
- For nativeplus/boundary issues, inspect `codegen_nativeplus.*` and runtime bridge/entrypoint code
- For OSR/JIT trigger bugs, inspect `scripts/interpreter.irt`, `runtime/interpreter/instruction_handler_base.h`, and `runtime/osr.cpp`
- For compiler-generated fastpath issues, also inspect `tests/checked/osr_irtoc_test.pa` and `tests/checked/compiler_to_interpreter.pa`
- generated outputs usually live in the build directory as `irtoc_code.cpp`, optional `irtoc_code_llvm.cpp`, and
  `disasm.txt`

## Testing

Useful targets:

- `ninja irtoc_unit_tests`
- `ninja irtoc_compiler_unit_tests`
- `ninja irtoc-interpreter-tests`

See also `tests/AGENTS.md` for interpreter/runner-based validation.

## Documentation

- `README.md` - basic build/context for irtoc
- `../docs/irtoc.md` - current irtoc pipeline, modes, validation, and debugging outputs
- `../docs/irtoc-intrinsics-for-interpreter.md` - interpreter-facing intrinsic generation details
- `../docs/flaky_debugging.md` - current repro and asm/IR localization workflow
- `../compiler/docs/README.md` - compiler/runtime boundary map when irtoc fast paths are involved
