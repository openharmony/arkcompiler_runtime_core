# Panda Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/panda
> Scope: panda/

## Purpose

Provides the **ark** executable (`ark_bin`) entry that wires together the runtime, compiler, verification, libpandabase, libpandafile, and ETS JS runtime to start the Ark VM, parse command-line options, load Panda files and stdlib, and execute bytecode or trigger AOT/verification. This module has only a single entry source and executable target; it does not implement runtime/compiler logic.

## Quick Start

### Build Commands

```bash
# Build ark executable
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/panda:ark_bin
```

## Where to Edit

### Module Entry Points

- `panda/panda.cpp` - Main entry: init runtime, parse args, load boot.abc/pandastdlib, run/verify
- `panda/BUILD.gn` - GN: ohos_executable("ark_bin"), produces ark executable
- `panda/CMakeLists.txt` - CMake: panda executable (when using CMake build tree)

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not implement logic that belongs in runtime/compiler/verification (e.g. config parsing) in this module; keep layering clear.
- ⚠️ Do not remove deps or configs just to "pass the build", or linking/headers will break.
- When adding CLI features, prefer extending compiler_options/base_options or existing pandargs; keep the entry point simple.

## Validation Checklist

### Module-Specific Test Commands

This module **has no** dedicated unit-test target; the ark executable is used indirectly by runtime/compiler/verification tests or integration tests.

### Module Dependencies

**Key dependencies**:
- `$ark_root/compiler:libarkcompiler` — Compiler library
- `$ark_root/libpandabase:libarkbase` — Base library
- `$ark_root/libpandafile:libarkfile` — Panda file format library
- `$ark_root/runtime:libarkruntime` — Runtime library

**Key downstream dependents**:
- static_core: includes `panda:arkts_bin` in host tools or device packages
- Product build scripts: install the **ark** executable on device/host to start the VM

## References

- Verified: BUILD.gn defines `ark_bin` (output_name = "ark"), install_enable = true
- Verified: Single executable target - no static/shared library defined
- Conditional: In static_core builds, `arkts_bin` may reference this executable
