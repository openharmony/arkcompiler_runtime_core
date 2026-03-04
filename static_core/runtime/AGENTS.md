# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: static_core/runtime
- **purpose**: Core runtime implementation including interpreter, GC, threading, class linking, JIT/AOT compiler interface, and architecture-specific code
- **primary language**: C++, Assembly

## About Runtime Core

The **runtime/** directory contains the core ArkVM runtime implementation:
- **Memory Management**: GC implementations (G1GC, Epsilon), allocators
- **Interpreter**: Bytecode interpreter
- **Compiler Interface**: JIT and AOT compiler orchestration
- **Threading**: ManagedThread, safepoints, stack walking
- **Class Linking**: Class loading, resolution, vtable/itable building
- **Bridges**: Interpreter-to-compiled code transitions

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
├── mem/                        # Memory Management
│   └── gc/
│       ├── g1/                # G1GC (main production GC)
│       └── epsilon-g1/         # Epsilon GC (no-op)
│
├── interpreter/                # Bytecode interpreter
│
├── arch/                       # Architecture-specific code
│
├── bridge/                     # Interpreter-compiler bridges
│
├── compiler.cpp/h             # Compiler task interface
├── runtime.cpp/h              # Runtime implementation
├── thread.cpp/h               # ManagedThread implementation
├── class_linker.cpp/h         # ClassLinker implementation
│
├── coroutines/                # Coroutine support
├── fibers/
├── entrypoints/               # Entrypoint
├── jit/                       # JIT compiler interface
├── tests/                     # Runtime unit tests
│
├── asm_defines/
├── CMakeLists.txt            # Build configuration
└── BUILD.gn                   # GN build
```

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
- Load .abc files
- Resolve classes and methods
- Build vtables, itables, imtables
- Class initialization
- App file loading

## Build Commands

See @../AGENTS.md

## Code Style

See @../AGENTS.md

## Testing

See @../AGENTS.md

## Dependencies

- **libarkbase**: OS abstraction, memory management
- **libarkfile**: Reading .abc files
- **irtoc**: Generates interpreter handlers
- **plugins/ets**: ETS language support
