# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: ETS (ArkTS-Sta) Plugin
- **purpose**: ArkTS-Sta language implementation for ArkVM, including compiler extensions, runtime support, standard library, and interop with JavaScript
- **primary language**: C++, ArkTS-Sta

## About ETS Plugin

The **ETS plugin** implements ArkTS-Sta language support for ArkVM:
- **Compiler**: ETS-specific compiler extensions and optimizations
- **Runtime**: Language runtime, coroutines, ANI (Ark Native Interface)
- **Standard Library**: ETS standard library (etsstdlib)
- **Interop**: ArkTS-Sta interop ArkTS-Dyn

## Directory Structure

Main File Directories

```
plugins/ets/
├── README.md                   # Documentation
├── CMakeLists.txt             # Build configuration
├── BUILD.gn                    # GN build
├── RegisterPlugin.cmake        # Plugin registration
├── HostTools.cmake            # Host tools configuration
│
├── playground/                 # Support playground
│
├── runtime/                    # ETS runtime implementation
│
├── sdk/                        # ETS sdk interface
│
├── compiler/                   # ETS compiler extensions
│
├── stdlib/                     # ETS standard library
│
├── tests/                      # ETS tests
│
├── assembler/                  # ETS assembler extensions
│
├── disassembler/               # ETS disassembler
│
├── abc2program/               # .abc to program converter
│
├── bytecode_optimizer/         # Bytecode optimization
│
├── dfx/                        # Diagnostic features
│
├── doc/                        # Documentation
│
└── cmake/                      # CMake scripts
```

## Build Commands

See @../../AGENTS.md

## File Formats

- **.ets**: ArkTS-Sta source code
- **.abc**: Compiled bytecode (ArkTS-Sta Bytecode)
- **.an**: AOT compiled code

## Code Style

See @../../AGENTS.md

## Testing

See @../../AGENTS.md
