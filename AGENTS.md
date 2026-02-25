# ArkCompiler Runtime Core

This repository contains the runtime component of OpenHarmony's ArkCompiler, including both dynamic (ArkTS-Dyn) and static (ArkTS-Sta) versions of the ArkTS language runtime.

## Project Overview

ArkCompiler Runtime Core provides:
- **Dynamic runtime** (ArkTS-Dyn): Located at root level with interpreter, JIT compiler, and AOT compilation
- **Static runtime** (ArkTS-Sta): Located in `static_core/` with LLVM-based static compilation
- **Compiler toolchain**: es2panda (source-to-bytecode), ark_aot (AOT compiler), and ark (VM)
- **Bytecode tools**: assembler, disassembler, optimizer, verifier
- **Libraries**: ABC file processing, runtime utilities, language plugins

## Directory Structure

```
├── assembler/              # Bytecode assembler: text (*.pa) -> binary (*.abc)
├── disassembler/           # Bytecode disassembler: binary (*.abc) -> text (*.pa)
├── bytecode_optimizer/     # Bytecode optimization tool
├── compiler/               # Compiler IR and optimization passes (dynamic)
├── verifier/               # Bytecode verifier (shared component)
├── irtoc/                  # IR-to-code tool for interpreter generation
├── isa/                    # Instruction Set Architecture definitions (YAML, templates)
│
├── libpandafile/           # Binary bytecode file (*.abc) format handling
├── libpandabase/           # Base utilities: logging, sync, data structures
├── libziparchive/          # ZIP archive operations
├── libabckit/              # ABC file inspection/modification library API
├── libark_defect_scan_aux/ # Security vulnerability scanner for bytecode files
│
├── ark_guard/              # Static bytecode obfuscation tool
├── abc2program/            # Bytecode to program converter
├── panda/                  # CLI VM tool for executing ABC files (dynamic)
├── pandastdlib/            # Standard library in Ark assembler format
│
├── static_core/            # Static runtime (ArkTS-Sta) - see subdir README
│   ├── compiler/           # Static compiler with LLVM backend
│   ├── runtime/            # Static runtime implementation
│   ├── plugins/ets/        # ETS (ArkTS) language plugin and stdlib
│   ├── libllvmbackend/     # LLVM backend integration
│   ├── verification/       # Bytecode verification (static)
│   ├── tests/              # Test suites: tests-u-runner, tests-u-runner-2
│   └── tools/              # Static runtime tools (es2panda, etc.)
│
├── common_runtime/         # Runtime components shared across versions
├── common_interfaces/      # Common interface definitions
├── platforms/              # Platform-specific utilities
├── arkplatform/            # Platform abstraction layer
│
├── plugins/                # Language plugins (dynamic runtime)
├── templates/              # Ruby templates for option/logger/code generation
├── taihe/                  # Build system integration
│
├── tests/                  # Test suites (dynamic)
├── test/                   # Additional test utilities
├── scripts/                # CI and build scripts
├── cmake/                  # CMake toolchain files and functions
├── gn/                     # GN templates and configs for OpenHarmony
│
└── docs/                   # Design documentation
    ├── bc_verification/    # Bytecode verification specification
    ├── file_format.md      # ABC binary file format
    ├── assembly_format.md  # Text assembly format
    ├── aot.md              # AOT compilation design
    └── ir_format.md        # Intermediate representation format
```

## Key Terminology

- **ABC file**: Binary bytecode file (Ark Byte Code) containing compiled ArkTS/ETS code
- **PA file**: Text assembly format for human-readable bytecode
- **AN file**: AOT compiled file for static execution

## Guidelines

- Build requirements: See `cmake/README.md`
- For detailed component documentation, refer to each subdirectory's README

## Build Systems

### GN (OpenHarmony Integration)
# For static_core (ArkTS-Sta)
```bash
# Bootstrap build
./scripts/build-panda-with-gn
```

Toolchain files are in `cmake/toolchain/` (CMake) and `gn/` (GN).

## Dynamic vs Static Runtime

The distinction between dynamic and static versions:

| Aspect | Dynamic (ArkTS-Dyn) | Static (ArkTS-Sta) |
|--------|---------------------|-------------------|
| Location | Root directories | `static_core/` |
| Compilation | Interpreter + optional JIT | LLVM AOT compilation |
| Primary Use | Runtime flexibility | Performance optimization |
| lib prefix | `libpanda*` | `libark*` |

## Component Relationships

```
Source Code (ETS/TS)
         ↓
    es2panda
         ↓
   ABC File (*.abc)
         ↓
    ┌────┴────┐
    ↓         ↓
 Dynamic   Static (ark_aot)
 Runtime   Compilation
    ↓           ↓
  AN File (*.an)
    ↓
 Execution by ark VM
```

## Documentation References

- File format: `docs/file_format.md`
- Assembly format: `docs/assembly_format.md`
- Bytecode verification: `docs/bc_verification/`
- IR format: `docs/ir_format.md`
- Static runtime: `static_core/README.md`
- libabckit API: `libabckit/doc/`
- Compiler docs: `compiler/docs/` and `static_core/compiler/docs/`

## Testing

- **Dynamic**: Use `tests/` and standard test runners
- **Static**: Use `static_core/tests/tests-u-runner/` or `tests-u-runner-2/`
- Run tests with: `ninja tests` or `ninja tests_full`

## Development Notes

- Only the `static_core` directory is for ArkTS-Sta, and the other directories serve ArkTS-Dyn.
- `runtime_core/static_core` is plugin-based. The `runtime_core/plugins/ets` directory hosts the unique features of ArkTS-Sta.
- The code comments in this repository should be written in English.
- The commit message should be written in English.
- Don't create commits directly. Have them reviewed.
