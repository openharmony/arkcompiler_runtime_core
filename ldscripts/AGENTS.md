# LdScripts Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/ldscripts
> Scope: ldscripts/

## Purpose

Provides **linker scripts** used on platforms that support 32-bit pointer optimization to place sections above 4GB so that Panda object addresses can use the lower 4GB. This directory **does not define GN or CMake targets**; it is only used by the CMake build via `-Wl,-T,<script>`.

## Where to Edit

### Module Entry Points

- `ldscripts/panda.ld` - Default linker script: section base 0x100010000 (~4GB+)
- `ldscripts/panda_test_asan.ld` - Linker script for AddressSanitizer: different base for ASAN mmap region

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not change script paths or filenames assumed in CMake (`panda.ld`, `panda_test_asan.ld`), or update `PandaCmakeFunctions.cmake` accordingly
- ⚠️ When changing section addresses or order, assess impact on 32-bit pointer optimization and ASAN
- ⚠️ Symbols like `LSFLAG_ASAN` in scripts must match CMake's `--defsym` usage

## Validation Checklist

### Module Dependencies

**Key upstream dependencies**:
- None - only relies on linker (LD) supporting the script syntax and the target platform ABI

**Key downstream dependents**:
- `cmake/PandaCmakeFunctions.cmake` - Selects script based on `PANDA_USE_32_BIT_POINTER`, `ARG_RAPIDCHECK_ON`, `PANDA_ENABLE_ADDRESS_SANITIZER`
- `static_core/cmake/PandaCmakeFunctions.cmake` - Same (if synced or copied from root cmake)

## References

- Verified: No BUILD.gn in this directory - does not participate in GN builds
- Verified: Used only when `PANDA_USE_32_BIT_POINTER` is true and target is not mobile/Windows/macOS/OHOS
- Verified: Scripts define section layout for `.interp`, `.note`, `.hash`, `.dynsym`, `.dynstr`, `.rela.*`, `.text`, `.rodata`, `.data`, `.bss`, etc.