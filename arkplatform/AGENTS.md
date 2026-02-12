# ArkPlatform Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/arkplatform
> Scope: arkplatform/

## Purpose

Provides platform-layer capabilities for interacting with the runtime and plugins. Centered on the **XGC** (eXtended GC) singleton and hybrid execution interfaces, used by components such as the ETS plugin via linking. Does not implement GC algorithms; only provides platform-side trigger and bridging APIs.

## Quick Start

### Build Commands

```bash
# Build shared library libarkplatform
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/arkplatform:libarkplatform

# Build static library
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/arkplatform:arkplatform_static
```

### Test Commands

```bash
# Run all runtime_core host unit tests (including ArkplatformTest)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core:runtime_core_host_unittest

# Run only arkplatform module unit tests
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/arkplatform/tests:host_unittest
```

## Where to Edit

### Module Entry Points

- `arkplatform/hybrid/xgc/xgc.cpp` - XGC singleton implementation
- `arkplatform/hybrid/xgc/xgc.h` - XGC class declaration (arkplatform::mem namespace)
- `arkplatform/BUILD.gn` - GN build configuration

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Singleton lifecycle (Create/Destroy/GetInstance) is agreed with callers (e.g. ETS runtime); changes must consider impact on plugins and static_core
- ⚠️ This library is linked dynamically or statically by plugins and static_core; API changes must consider compatibility

## Validation Checklist

### Module-Specific Test Commands

The GN test target `ArkplatformTest` links `libarkplatform` and includes:
- `xgc_test.cpp` - XGC singleton and Trigger behavior

### Module Dependencies

**Key upstream dependencies**:
- None - depends only on sources in this directory and build configs

**Key downstream dependents**:
- `static_core/plugins/ets` - links `libarkplatform` for VM and platform bridging
- Test targets: `arkplatform_host_unittest`, `arkplatform_unittest`

## References

- Verified: Single-source library with only `hybrid/xgc/xgc.cpp` implementation
- Verified: No code generation pipeline
- Conditional: Mirror BUILD.gn exists under static_core build tree
