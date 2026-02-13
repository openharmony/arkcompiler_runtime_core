# CMake Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/cmake
> Scope: cmake/

## Purpose

Provides **CMake build infrastructure**, including global definitions, code style and static checks, testing, host tools, assembly and code generation, toolchain and third-party support. This directory **does not define GN targets**; it is only used by the CMake build tree via `include()` or `add_subdirectory()`.

## Quick Start

### Build Commands

This directory is not built on its own; under a CMake build tree:

```bash
mkdir build && cd build
cmake ${panda_dir}
make
```

### Style Check Commands

```bash
make clang_format
make clang_tidy
make clang_force_format
```

## Where to Edit

### Module Entry Points

- `cmake/Definitions.cmake` - Global variables and definitions
- `cmake/PandaCmakeFunctions.cmake` - Panda custom CMake functions
- `cmake/ClangTidy.cmake` - clang-tidy integration
- `cmake/CodeStyle.cmake` - Code style targets
- `cmake/Testing.cmake` - Test framework integration
- `cmake/toolchain/` - Toolchain and cross-compilation configs

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not add BUILD.gn or GN logic in this directory; it serves only the CMake build
- ⚠️ Do not break variables, target names, or interfaces used by static_core or subprojects (e.g. PANDA_ROOT in Definitions)
- ⚠️ Maintain compatibility with existing `include()` callers in static_core/CMakeLists.txt

## Validation Checklist

### Module-Specific Test Commands

This directory does not define test targets; `Testing.cmake` and `CommonTesting.cmake` provide rules for other modules.

### Module Dependencies

**Key upstream callers**:
- `static_core/CMakeLists.txt` - Includes Definitions, PandaCmakeFunctions, HostTools, etc.

**Key upstream dependencies**:
- CMake 3.14+, compiler (Clang/GCC)
- Optional: clang-format, clang-tidy, Doxygen, ccache

## References

- Verified: No BUILD.gn in this directory - CMake-only module
- Verified: Used by static_core via include() statements for build infrastructure
- Conditional: Third-party paths come from PANDA_THIRD_PARTY_* or ark-third-party
