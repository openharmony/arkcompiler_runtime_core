# PandaStdLib Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/pandastdlib
> Scope: pandastdlib/

## Purpose

Maintains **Panda standard library assembly source** (`pandastdlib.pa`): defines base types and exception classes required by runtime (e.g. `panda.String`, `panda.Object`, `panda.StackOverflowException`, `Math`, `System`), compiled by assembler to `arkstdlib.abc` and loaded by runtime as boot standard library.

## Quick Start

### Build Commands

```bash
# Build arkstdlib.abc (in a build tree that includes static_core)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/static_core/pandastdlib:arkstdlib
```

The build reads `arkcompiler/runtime_core/pandastdlib/pandastdlib.pa` and uses `ark_asm` to generate `arkstdlib.abc`.

## Where to Edit

### Module Entry Points

- `pandastdlib/pandastdlib.pa` - Stdlib assembly: Record definitions (Object/Class/String, exceptions, Math/System/IO, etc.)
- `pandastdlib/CMakeLists.txt` - CMake: add_panda_assembly(arkstdlib)

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not break Record names, field layout, or inheritance expected by bytecode/runtime
- ⚠️ Do not add a GN BUILD.gn here that redefines arkstdlib; that would conflict with static_core/pandastdlib

## Validation Checklist

### Module Dependencies

**Key upstream dependencies**:
- **Assembler** - CMake uses `add_panda_assembly` (needs ark_asm); GN uses `ark_asm_gen` template and assembler toolchain

**Key downstream dependents**:
- **static_core/pandastdlib** - GN build references `${ark_root}/pandastdlib/pandastdlib.pa` as input
- **static_core/runtime/tests** - Depends on `pandastdlib:arkstdlib` as boot library for runtime tests
- **Runtime** - Loads `arkstdlib.abc` to provide standard types and exceptions for bytecode and AOT

## References

- Verified: No GN BUILD.gn in this directory - GN build is in static_core/pandastdlib
- Verified: Single source file `pandastdlib.pa` for version and ABI consistency
- Verified: Dual output names - CMake uses `pandastdlib.bin` symlink compatible with `arkstdlib.abc`