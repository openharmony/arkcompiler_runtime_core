# Plugins Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/plugins
> Scope: plugins/

## Purpose

Maintains `plugins/` directory as **language/feature plugin** container. Under runtime_core it mainly contains **ecmascript** plugin (assembler extensions, metadata, etc.) and its tests. The root BUILD.gn includes each enabled plugin's `ark_packages` in `ark_packages` and `ark_host_linux_tools_packages` via `enabled_plugins`.

## Quick Start

### Build Commands

```bash
# Build all ark_packages (including enabled plugins' ark_packages)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core:ark_packages

# Run only plugins unit tests (ecmascript)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/plugins/ecmascript/tests:host_unittest
```

## Where to Edit

### Module Entry Points

- `plugins/ecmascript/` - ECMAScript plugin (assembler extensions, metadata)
- `plugins/ecmascript/tests/BUILD.gn` - GN test target PluginsTest

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not break how root BUILD.gn enumerates `plugins/$plugin:ark_packages`
- ⚠️ Each plugin must provide targets referenced by root BUILD.gn (e.g. `ark_packages`)

## Validation Checklist

### Module-Specific Test Commands

- **PluginsTest** (`plugins/ecmascript/tests/BUILD.gn`): `host_unittest_action`, source `ecmascript_meta_test.cpp`

### Module Dependencies

**Key upstream dependencies**:
- `assembler:libarkassembler_static` - For plugin assembler extensions
- `libpandabase:libarkbase_static` - Base library

**Key downstream dependents**:
- **Root BUILD.gn** - `ark_packages` depends on `plugins/$plugin:ark_packages` via `foreach(plugin, enabled_plugins)`
- **assembler** - References `plugins/ecmascript/assembler` via `enabled_plugins`
- **static_core** - runtime, compiler, disassembler, etc. import `plugins.gni` or depend on plugin-generated headers

## References

- Verified: Plugin enumeration via root BUILD.gn using `foreach(plugin, enabled_plugins)`
- Verified: Each plugin must provide `ark_packages` (and optionally `ark_host_linux_tools_packages`)
- Verified: ecmascript plugin provides assembler extensions and metadata