# Platforms Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/platforms
> Scope: platforms/

## Purpose

Maintains `platforms/` platform-abstraction source tree: provides OS implementations for Unix/Windows that pair with `libpandabase` (file, thread, mutex, error handling, memory, dynamic library loading, etc.), and runtime helpers under common/ohos (thread, app install verification, etc.).

## Quick Start

### Build Commands

This directory is not built as a separate library; its sources are compiled with `libpandabase` target. To verify logic:

```bash
# Run all runtime_core host unit tests (including PlatformsTest)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core:runtime_core_host_unittest

# Run only platforms module unit tests
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/platforms/tests:host_unittest
```

## Where to Edit

### Module Entry Points

- `platforms/unix/libpandabase/` - Unix/Linux implementation files
- `platforms/windows/libpandabase/` - Windows implementation files
- `platforms/common/` - Common runtime helpers
- `platforms/ohos/` - OHOS platform GN config

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ When changing interfaces or behavior, keep Unix and Windows implementations semantically consistent
- ⚠️ Do not delete or rename referenced source files without updating `libpandabase/BUILD.gn`

## Validation Checklist

### Module-Specific Test Commands

The GN test target `PlatformsTest` links `libarkbase_static`. Test source:
- `file_test.cpp` - Cross-platform File behavior (uses platform's File implementation per platform)

### Module Dependencies

**Key upstream dependencies**:
- None - platform implementations are included in `libpandabase`

**Key downstream dependents**:
- **libpandabase** - References platform sources via `libarkbase_sources` in `libpandabase/BUILD.gn`
- **platforms/tests** - Depends on `libpandabase:libarkbase_static` for testing

## References

- Verified: No standalone library target - sources are referenced via `libpandabase/BUILD.gn`
- Verified: Platform branches for Unix/Windows implementations selected by build conditions
- Verified: Header exposure via `libpandabase`'s `include_dirs` includes `$ark_root/platforms`