# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Ark Compiler Runtime Core Static Core
- **purpose**: Virtual machine runtime designed for OpenHarmony, providing bytecode execution, JIT/AOT compilation, and garbage collection for the ArkTS-Sta programming language
- **primary language**: C++, ArkTS

## About Static Core

This is the **Ark Runtime Core** (static_core) - a virtual machine runtime designed for OpenHarmony. It supports the ArkTS-Sta programming language through the ETS plugin. The runtime includes a bytecode interpreter, JIT compiler, AOT (Ahead-Of-Time) compiler, and multiple garbage collector implementations.

## Directory Structure

Main File Directories

```
static_core/
├── runtime/                   # Core runtime implementation
│   ├── include/               # Public headers (class_linker.h, thread.h, object_header.h, etc.)
│   ├── interpreter/           # Bytecode interpreter (irtoc handlers, frame, state)
│   ├── mem/                   # Memory management
│   ├── arch/                  # Architecture-specific code (ARM, RISC-V, x86_64)
│   ├── jit/                   # Profile support
│   ├── coroutines/            # Coroutine support
│   ├── entrypoints/           # Generated runtime intrinsics 
│   ├── compiler.cpp/h         # Compiler
│   ├── runtime.cpp/h          # Main runtime implementation
│   ├── thread.cpp/h           # Thread management
│   ├── class_linker.cpp/h     # Class loading and linking
│   └── tests/                 # Runtime unit tests
│
├── compiler/                  # Compiler
│
├── plugins/                   # Language plugins
│   └── ets/                   # ETS (ArkTS) language plugin
│
├── irtoc/                     # IR-to-Code compiler
│
├── isa/                       # Instruction set architecture
│   ├── isa.yaml               # Bytecode instruction definitions
│   └── templates/             # Instruction templates
│
├── libarkbase/                # Base library
│   ├── os/                    # OS abstraction layer
│   ├── mem/                   # Memory utilities
│   └── utils/                 # Utility functions
│
├── libarkfile/                 # File handling library
├── libllvmbackend/             # LLVM backend (optional)
├── libziparchive/              # ZIP archive support
│
├── assembler/                  # Panda assembler
├── disassembler/               # Bytecode disassembler
├── bytecode_optimizer/         # Bytecode optimization tool
├── verifier/                   # Bytecode verifier
│
├── tests/                      # Test suites
│
├── third_party/                # Third-party dependencies
│
├── scripts/                    # Build and utility scripts
│
├── cmake/                      # CMake build configuration
│
├── docs/                       # Architecture documentation
│
└── platforms/                 # Platform-specific configurations
    ├── unix/                  # Unix-like systems
    └── windows/               # Windows platform
```

## Build Commands

### Build Release Mode
```bash
cd runtime_core/static_core
cmake -B out -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain/host_clang_default.cmake -GNinja .
cmake --build out
```

### Build Modes
- `Debug` - Assertions enabled, no optimizations, debug info
- `Release` - No assertions

### Running Tests
```bash
cd runtime_core/static_core/out

# Run all test suites
ninja tests
```

### Compile && Run ETS File

#### Test Case
```ts
//test.ets

function main() {
  console.log("hello");
}
```

#### Run ETS File
```bash
cd runtime_core/static_core

# Compile ets file to abc file
./out/bin/es2panda test.ets --output test.abc

# run abc file with ark
./out/bin/ark --boot-panda-files=./out/plugins/ets/etsstdlib.abc --load-runtimes=ets test.abc test.ETSGLOBAL::main
```

## High-Level Architecture

### Core Runtime Components

**1. Memory Management (`runtime/mem/`)**
- **Object Header**: Object header design supporting 64/128-bit configurations
- **GC Implementations**:
  - `g1/` - G1GC (Garbage First) - region-based generational GC (main production GC)
  - `epsilon-g1/` - Epsilon GC - no-op GC
- **Allocators**: Arena allocator, humongous allocator for large objects, TLAB support

**2. Interpreter (`runtime/interpreter/`)**
- **Stackless interpreter**: No new host frame created for managed-to-managed calls
- **Indirect threaded dispatch**: Uses computed goto for fast bytecode dispatch
- **Register-based bytecode**: Virtual registers with accumulator for compact encoding
- **Irtoc**: IR-to-Code compiler that generates optimized interpreter handlers from IR descriptions

**3. Compiler Infrastructure (`compiler/`)**
- **AOT Compiler** (`compiler/aot/`): Compiles bytecode to native code stored in `.an` ELF files
- **JIT Compiler**: Runtime compilation with hotness-based triggering

**4. Class Linking (`runtime/include/class_linker.h`)**
- Loads and resolves classes from `.abc` (Ark Bytecode) files
- Builds vtables, itables (interface tables), and imtables
- Handles class initialization and verification

**5. Thread Management (`runtime/include/thread.h`)**
- ManagedThread/MTManagedThread: Per-thread runtime state
- Safepoint implementation for GC stop-the-world
- Stack walking for GC root scanning

## Code Style

- 4 spaces indent, 120 character line length
