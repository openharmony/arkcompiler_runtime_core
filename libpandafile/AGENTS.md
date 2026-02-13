# LibPandaFile Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/libpandafile
> Scope: libpandafile/

## Purpose

Provides read/write support for ARK `.abc` binaries (File/FileReader/FileWriter/FileItemContainer), bytecode instruction definitions and emission (BytecodeInstruction/BytecodeEmitter), and metadata accessors for class/method/field/annotation/debug info, etc. (*DataAccessor); maintains ISA bytecode and type-system code-generation pipeline.

## Quick Start

### Build Commands

```bash
# Build shared library libarkfile
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/libpandafile:libarkfile

# Build static library
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/libpandafile:libarkfile_static

# Build frontend toolchain static library
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/libpandafile:libarkfile_frontend_static
```

### Test Commands

```bash
# Run all runtime_core host unit tests (including LibPandaFileTest)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core:runtime_core_host_unittest

# Run only libpandafile module unit tests
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/libpandafile/tests:host_unittest
```

## Where to Edit

### Module Entry Points

- `libpandafile/BUILD.gn` - GN: libarkfile (static/shared) + ISA/type code-gen rules
- `libpandafile/pandafile_isapi.rb` - ISA extension script (bytecode emission helpers)
- `libpandafile/types.yaml` - Type system definition data source
- `libpandafile/types.rb` - Type system generation script
- Source implementations: `libpandafile/file.{h,cpp}`, `libpandafile/bytecode_emitter.{h,cpp}`, various `*DataAccessor` classes

### Module-Specific "Do Not Edit" Boundaries

⚠️ **Critical**: This module uses **code generation** (Ruby + ERB + YAML).
- ⚠️ Do not edit generated files in build directory (e.g. `build/**/bytecode_instruction_enum_gen.h`, `build/**/type.h`). Edit source and regenerate.
- ⚠️ Do not modify `isa/*.yaml` (global ISA definition) - affects all modules repo-wide

### Code-Generation Rules

- **ISA-driven**: Bytecode instruction enums, emitters, and inline implementations come from `templates/*.erb` + `pandafile_isapi.rb` + `isa/*.yaml`
- **Type-system driven**: `type.h` is generated from `types.yaml` + `types.rb` + `templates/type.h.erb`
- **DataAccessor pattern**: Each kind of metadata in `.abc` has a corresponding `*DataAccessor` that works directly on binary offsets

## Validation Checklist

### Module-Specific Test Commands

The GN test target `LibPandaFileTest` links `libarkfile_static` and `libarkassembler_static`. Test sources include:
- `bytecode_emitter_tests.cpp`, `file_test.cpp`, `file_items_test.cpp`
- `debug_info_extractor_test.cpp`, `pgo_test.cpp`

### Module Dependencies

**Key upstream dependencies**:
- `libpandabase/` - Base library (logging, utilities, data structures)
- `libziparchive/` - ZIP archive library for reading `.abc` from `.zip` / `.hap`
- `$ark_root/isa/` - ISA definition for bytecode code generation

**Key downstream dependents**:
- `assembler/` - Assembler (links `libarkfile_static` / `libarkfile_frontend_static`)
- `compiler/` - Compiler
- `bytecode_optimizer/` - Bytecode optimizer
- `verifier/` - Verifier (links `libarkfile_static_verifier`)

## References

- Verified: BUILD.gn defines static/shared library targets and code-generation rules
- Verified: ISA-driven and type-system driven code generation using Ruby + ERB + YAML
- Verified: DataAccessor pattern for zero-copy access to binary .abc file metadata