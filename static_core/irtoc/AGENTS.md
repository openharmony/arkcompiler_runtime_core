# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Irtoc
- **purpose**: Used to generate optimized interpreter handlers for the ArkVM bytecode interpreter and used for intrinsics also
- **primary language**: Ruby, C++

## About Irtoc

**Irtoc** It is used to generate interpreter handlers for the ArkVM bytecode interpreter and used for intrinsics also.

## Directory Structure

Main File Directories

```
irtoc/
├── backend/                    # code generation backend
│
├── lang/                       # Ruby IR language definition
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
├── CMakeLists.txt             # CMake build configuration
└── README.md                   # Basic documentation
```

## Build Commands

see @../AGENTS.md

## IR Language (.irt files)

The IR language is a Ruby-based DSL for describing bytecode handler logic.

## Testing

see @../AGENTS.md
