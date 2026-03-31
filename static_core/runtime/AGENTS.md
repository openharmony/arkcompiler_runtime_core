# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: static_core/runtime
- **purpose**: Core runtime implementation including interpreter, GC, threading, class linking, JIT/OSR/AOT compiler integration, deoptimization, and architecture-specific code
- **primary language**: C++, Assembly

## About Runtime Core

The **runtime/** directory contains the core ArkVM runtime implementation:

- **Memory Management**: GC implementations (G1GC, Epsilon), allocators
- **Interpreter**: Bytecode interpreter
- **Compiler Interface**: JIT, OSR, and AOT orchestration
- **Threading**: ManagedThread, safepoints, stack walking
- **Class Linking**: Class loading, resolution, vtable/itable building
- **Bridges**: Interpreter-to-compiled transitions, deoptimization, runtime entrypoints

## Directory Structure (Key Components)

Main File Directories

```
runtime/
├── include/                    # Public headers
│   ├── runtime.h              # Main Runtime class
│   ├── thread.h               # ManagedThread interface
│   ├── class_linker.h         # ClassLinker interface
│   ├── object_header.h        # Object header design
│   ├── coretypes/
│   │   └── ...
│   └── ...
│
├── mem/                        # Memory management
│   └── gc/
│       ├── g1/                # G1-based collector
│       ├── epsilon/           # Minimal GC for debug/perf experiments
│       └── epsilon-g1/        # Alternate G1-shaped GC variant
│
├── interpreter/                # Bytecode interpreter
├── arch/                       # Architecture-specific code
├── bridge/                     # Interpreter-compiler bridges
├── compiler.cpp/h             # Runtime-side compiler orchestration and RuntimeInterface implementation
├── compiler_queue_*           # JIT scheduling strategies
├── compiler_*worker.*         # Background compiler workers / task manager integration
├── osr.cpp/h                  # On-stack replacement entry and frame reconstruction
├── deoptimization.cpp/h       # Compiled-to-interpreter transition
├── intrinsics.cpp             # Runtime intrinsics used by compiled code
├── runtime.cpp/h              # Runtime implementation
├── thread.cpp/h               # ManagedThread implementation
├── class_linker.cpp/h         # ClassLinker implementation
│
├── coroutines/                # Coroutine support
├── fibers/
├── entrypoints/               # Generated runtime entrypoints used by compiler/irtoc
├── jit/                       # Profiling data and `.ap` profile persistence, not task scheduling
├── tests/                     # Runtime unit tests
│
├── asm_defines/
├── CMakeLists.txt            # Build configuration
└── BUILD.gn                   # GN build
```

## Compiler Integration Map

For compiler debugging, do not treat `runtime/jit/` as the whole JIT subsystem:

- `compiler.cpp/h` contains runtime-side compilation entry, task submission, code installation, and the main `RuntimeInterface` used by the compiler
- `compiler_queue_*` and `compiler_*worker.*` implement background compilation queues and workers
- `osr.cpp/h` reconstructs frames and jumps from the interpreter into compiled OSR code
- `deoptimization.cpp/h` handles reverse transitions from compiled code back to the interpreter
- `entrypoints/entrypoints.yaml`, generated entrypoint files, and `intrinsics.cpp` define the compiler-visible runtime call surface; `runtime.yaml` describes runtime intrinsics metadata
- `bridge/` contains architecture-specific interpreter/compiled/runtime bridge assembly and is the first stop for calling-convention and frame-transition bugs
- `jit/` stores profiling data (`ProfilingData`, inline caches, branch/throw counters) and `.ap` serialization/loading
- `interpreter/` updates hotness counters and can trigger JIT/OSR requests during execution

## Interpreter Selection and JIT Caveats

- `runtime/options.yaml` currently declares `--interpreter-type` with default `llvm`
- the runtime may downgrade that default to `irtoc` or `cpp` depending on build flags and runtime restrictions; see `runtime/interpreter/interpreter_impl.cpp`
- dynamic-language execution defaults to `cpp` unless `--interpreter-type` was set explicitly
- for reproducible bugs, pass `--interpreter-type=cpp|irtoc|llvm` explicitly instead of relying on implicit defaults
- `--no-async-jit=true` is the force-jit/testing path; `Runtime::GetGCType` forces STW GC unless epsilon, and the runtime rejects `--no-async-jit=true` together with loaded AOT files

## Core Subsystems

### 1. Memory Management (`mem/gc/`)

**G1GC** (main production GC):

- Region-based generational collector
- Concurrent marking
- Compaction
- TLAB (Thread-Local Allocation Buffers)

### 2. Threading (`thread.cpp/h`)

**ManagedThread**:

- Per-thread runtime state
- Safepoint implementation for GC stop-the-world
- Stack walking for root scanning
- Thread local storage

### 3. Class Linking (`class_linker.cpp/h`)

**ClassLinker**:

- Load `.abc` files
- Resolve classes and methods
- Build vtables, itables, imtables
- Class initialization
- App file loading

### 4. Compiler/Runtime Boundary

- JIT hotness threshold and OSR enablement are runtime options in `runtime/options.yaml`
- compiler-facing safety/debug options in `runtime/options.yaml` include `verify-entrypoints`, `compiler-enable-jit`, `compiler-enable-osr`, and stack-walker verification
- `runtime/compiler.cpp` contains GC-dependent fastpath checks and runtime queries consumed by `compiler/optimizer/ir/runtime_interface.h`
- interpreter hot loops trigger OSR through logic in `runtime/interpreter/instruction_handler_base.h`
- OSR entry and frame materialization happen in `runtime/osr.cpp`
- compiled-code stack walking and deoptimization rely on `code_info/`, `stack_walker.cpp`, and deopt/runtime entrypoints

## FastPath Design Notes

- Follow the repo-level `irtoc` policy in `../AGENTS.md` and the irtoc-specific guidance in `../irtoc/AGENTS.md`
- Runtime-side fastpath work usually needs coordinated changes across `runtime/compiler.cpp`, entrypoint metadata, bridge or frame code, and sometimes `irtoc/`; avoid treating any one file as the whole change

## Calling Convention and Stack Walking

Compiler bugs often surface here even when the root cause is outside `runtime/`:

- `runtime/include/stack_walker.h` and `stack_walker.cpp` interpret compiler-produced `code_info` stack maps
- `runtime/bridge/` contains c2i, i2c, compiled-to-runtime, and deoptimization bridge assembly
- `runtime/entrypoints/entrypoints.yaml` and generated entrypoint files define the ABI-visible runtime call surface
- `runtime/tests/stack_walker_test.cpp` validates stack-map and vreg reconstruction behavior
- `runtime/tests/c2i_bridge_test.cpp` and `runtime/tests/i2c_bridge_test.cpp` validate calling convention and frame transitions

## Debugging Hints

- If a compiler change breaks execution only after installation, inspect `runtime/compiler.cpp`, `runtime/osr.cpp`, `deoptimization.cpp`, and `entrypoints/`
- Fastpath issues often cross `runtime/compiler.cpp`, `entrypoints/`, `compiler/optimizer/code_generator/`, and `irtoc/`
- `StackWalker` is central for deoptimization, exceptions, OSR, and debugging compiled frames
- For profile-guided issues, inspect `runtime/jit/` and `compiler/tools/aptool/`
- For bridge or ABI issues, inspect `runtime/bridge/`, `runtime/tests/c2i_bridge_test.cpp`, `runtime/tests/i2c_bridge_test.cpp`, and compiler callconv gtests
- Use `StackWalker::Dump(...)`, `verify-entrypoints`, and compiler IR/disasm dumps together when a bug crosses codegen and runtime frames
- For AOT entry-installation issues, `runtime/class_linker.cpp` still contains `MaybeLinkMethodToAotCode`, which is a
  useful breakpoint together with `--log-level=debug --log-components=aot`
- For GC-sensitive runtime failures, add `--log-level=debug --log-components=gc:mm-obj-events` and
  `--log-detailed-gc-info-enabled=true`

## Documentation

- `../docs/runtime-compiled_code-interaction.md` - calling convention, stack-frame layout, bridges, runtime calls
- `../docs/code_metainfo.md` - stack maps / inline info consumed by `StackWalker`
- `../docs/deoptimization.md` - deoptimization flow and recovery
- `../docs/on-stack-replacement.md` - OSR trigger and frame replacement
- `../docs/flaky_debugging.md` - current repro, logging, and asm/IR localization workflow
- `../compiler/AGENTS.md` - compiler-side first stops and compiler-owned documentation

## Build Commands

See @../AGENTS.md

## Code Style

See @../AGENTS.md

## Testing

See @../AGENTS.md

## Dependencies

- **libarkbase**: OS abstraction, memory management
- **libarkfile**: Reading `.abc` files
- **irtoc**: Generates interpreter handlers
- **plugins/ets**: ETS language support
- **compiler/**: Optimizing compiler and code generator
