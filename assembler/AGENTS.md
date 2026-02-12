# Assembler Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/assembler
> Scope: assembler/

## Purpose

Provides the Panda assembly IR model (Program/Function/Record/Ins, etc.), lexing/parsing and bytecode emission, and fixes for parsing/emission/metadata issues; maintains the ISA and metadata code-generation pipeline.

## Quick Start

### Build Commands

```bash
# Build shared library libarkassembler
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/assembler:libarkassembler

# Build static library
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/assembler:libarkassembler_static
```

### Test Commands

```bash
# Run all runtime_core host unit tests (including AssemblerTest)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core:runtime_core_host_unittest

# Run only assembler module unit tests
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/assembler/tests:host_unittest
```

## Where to Edit

### Module Entry Points

- `assembler/BUILD.gn` - GN build configuration with code-gen rules
- `assembler/assembly-emitter.{h,cpp}` - Bytecode emitter: writes Program to panda_file binary (.abc)
- `assembler/assembly-parser.{h,cpp}` - Assembly syntax parser: Token → Program
- Code generation sources: `metadata.yaml`, `asm_isapi.rb`, `asm_metadata.rb`, `templates/*.erb`

### Module-Specific "Do Not Edit" Boundaries

⚠️ **Critical**: This module uses **code generation** (Ruby + ERB + YAML).
- ⚠️ Do not edit generated files in build directory (e.g. `build/**/meta_gen.h`, `build/**/ins_to_string.cpp`). Edit source and regenerate.
- ⚠️ For ISA/metadata changes, prefer extending data sources and templates; avoid hard-coding duplicate tables in hand-written C++

### Code-Generation Rules

- **ISA-driven**: Instruction-related API/print/parser helpers come from `templates/*.erb` + `asm_isapi.rb` + `isa/*.yaml`
- **Metadata-driven**: Attributes/annotations come from `metadata.yaml` + `asm_metadata.rb` + `templates/meta_gen.cpp.erb`
- When adding instructions or attributes: update data files, templates, and unit tests that cover the new behavior

## Validation Checklist

### Module-Specific Test Commands

The GN test target `AssemblerTest` links the `libarkassembler` shared library. Key test sources:
- `annotation_test.cpp`, `assembler_emitter_test.cpp`, `assembler_ins_test.cpp`
- `assembler_lexer_test.cpp`, `assembler_mangling_test.cpp`, `assembler_parser_test.cpp`
- `assembler_access_flag_test.cpp`, `ecmascript_meta_test.cpp`

### Module Dependencies

**Key upstream dependencies**:
- `libpandafile/` - ARK `.abc` file format and APIs
- `libpandabase/` - Base library (logging, utilities, data structures)
- `../libpandafile/pandafile_isapi.rb` - ISA parsing helper script

**Key downstream dependents**:
- `bytecode_optimizer/` - Bytecode optimization
- `compiler/` - Compiler
- `disassembler/` - Disassembler (`ark_disasm`)
- `abc2program/` - .abc → pandasm Program conversion (`abc2prog`)

## References

- Verified: BUILD.gn defines static/shared library targets and code-generation rules
- Verified: Code-generation pipeline uses Ruby + ERB + YAML for ISA and metadata
- Conditional: GN tests require `es2abc` to produce `.abc` files for `assembler_access_flag_test`
