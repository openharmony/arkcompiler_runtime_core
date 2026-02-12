# ISA Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/isa
> Scope: isa/

## Purpose

Provides **single source of truth** for Panda bytecode ISA (`isa.yaml`) and **merge-and-generate infrastructure**. The `isa_combine` step merges core `isa.yaml` with plugin ISAs into `$root_gen_dir/isa/isa.yaml`, used by repo-wide `ark_isa_gen`, `ark_gen_file`, and by interpreter, compiler, assembler, disassembler, etc.

## Quick Start

### Build Commands

```bash
# Build ISA merge output (usually built automatically when a dependent target is built)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/isa:isa_combine
```

## Where to Edit

### Module Entry Points

- `isa/BUILD.gn` - GN: isa_combine (action), isa_combine_etc (prebuilt_etc)
- `isa/isa.yaml` - Core ISA definition (instructions, registers, calling convention)
- `isa/combine.rb` - Merge script: core isa.yaml + plugin ISAs → $root_gen_dir/isa/isa.yaml
- `isa/gen.rb` - Template generation driver
- `isa/isapi.rb` - ISA query API (used by ERB templates and plugin isapi)

### Module-Specific "Do Not Edit" Boundaries

⚠️ **Critical**: This module is **root of repo-wide ISA and bytecode generation pipeline**.
- ⚠️ Do not maintain instruction tables or encodings in hand-written C++ that duplicate isa.yaml; generate via templates and isapi
- ⚠️ Do not break `combine.rb`'s input/output contract (merge order, YAML structure), or all `ark_isa_gen` consumers will break
- ⚠️ When changing instruction set or encoding format, document in `docs/changelogs/` and assess compatibility

### Code-Generation Rules

- Add or change instructions or formats only in `isa.yaml`
- After changing ISA, regenerate repo-wide and run related unit tests
- Update `docs/` and changelogs when needed

## Validation Checklist

### Module Dependencies

**Key upstream dependencies**:
- Plugins - BUILD.gn obtains each plugin's ISA path via `srcs_isa_path`
- Ruby - `combine.rb`, `gen.rb`, and `isapi.rb` require Ruby at build time

**Key downstream dependents**:
- `ark_config.gni` - `ark_isa_gen` depends on `isa:isa_combine`
- `libpandafile` - Depends on `isa_combine` via `ark_isa_gen`
- `assembler` - Indirectly depends on isa.yaml and isapi via ISA templates
- `static_core` - Many targets have `ark_isa_gen` or generation rules that depend on `isa:isa_combine`

## References

- Verified: BUILD.gn defines `isa_combine` action and `isa_combine_etc` prebuilt target
- Verified: Single-source `isa.yaml` as core ISA definition
- Verified: Plugin extension mechanism that merges plugin ISAs with core ISA
