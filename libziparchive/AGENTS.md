# LibZipArchive Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/libziparchive
> Scope: libziparchive/

## Purpose

A C++ wrapper around zlib/minizip that provides APIs for creating, locating, reading, and extracting ZIP files, used by `libpandafile` and others to load `.abc` files from `.zip` / `.hap` packages. This module is a pure C++ wrapper with no code generation.

## Quick Start

### Build Commands

```bash
# Build shared library libarkziparchive
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/libziparchive:libarkziparchive

# Build static library
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/libziparchive:libarkziparchive_static

# Build frontend toolchain static library
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/libziparchive:libarkziparchive_frontend_static
```

### Test Commands

```bash
# Run all runtime_core host unit tests (including LibZipArchiveTest)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core:runtime_core_host_unittest

# Run only libziparchive module unit tests
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/libziparchive/tests:host_unittest
```

## Where to Edit

### Module Entry Points

- `libziparchive/BUILD.gn` - GN: libarkziparchive (static/shared)
- `libziparchive/zip_archive.h` - ZIP archive API (ZipArchiveHandle/EntryFileStat/GlobalStat, etc.)
- `libziparchive/zip_archive.cpp` - ZIP archive implementation (minizip wrapper)

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not relax or bypass checks to "pass tests" (e.g. disabling clang-tidy or skipping format checks)
- ⚠️ Do not modify the zlib/minizip third-party code itself (under `third_party/zlib`); use only its public API

## Validation Checklist

### Module-Specific Test Commands

The GN test target `LibZipArchiveTest` links `libarkziparchive_static`, `libarkfile_static`, and `libarkassembler_static`. Test source:
- `libziparchive_tests.cpp` - ZIP file creation, extraction, error handling

### Module Dependencies

**Key upstream dependencies**:
- `libpandabase/` - Base library (logging, utilities)
- `zlib:libz` / `zlib_deps` - Zlib compression (minizip dependency)

**Key downstream dependents**:
- `libpandafile/` - Panda file format library (links `libarkziparchive_static` to load `.abc` from ZIP)
- `tests/fuzztest/` - Multiple fuzz tests for ZIP archive operations

## References

- Verified: Single source file `zip_archive.cpp` as C++ wrapper over zlib/minizip
- Verified: No code generation pipeline in this module
- Verified: Primarily a dependency of `libpandafile` for reading `.abc` from ZIP containers