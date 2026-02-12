# GN Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/gn
> Scope: gn/

## Purpose

Provides **scripts and helpers** for **GN builds**, used by modules' `action()` or generation rules (e.g. version file generation `make_version_file.sh`, Git info `find_git.sh`, CMake-style config `cmake_configure_file.py`). This directory **does not define GN targets**; it is only referenced from other BUILD.gn or .gni via `script = "$ark_root/gn/build/..."`.

## Where to Edit

### Module Entry Points

- `gn/build/make_version_file.sh` - Generates version header (Git hash, etc.)
- `gn/build/find_git.sh` - Finds Git executable or repo info
- `gn/build/cmake_configure_file.py` - Python impl of CMake-style configure_file

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not break calling convention of existing actions (argument count/order, output path and content format), or version file generation will fail
- ⚠️ Do not add BUILD.gn or define targets in this directory; it is a script-only directory

## Validation Checklist

### Module Dependencies

**Key upstream dependencies**:
- `make_version_file.sh` needs `git` (optional), `bash`
- `cmake_configure_file.py` needs Python

**Key downstream dependents**:
- `libpandabase/BUILD.gn` - `action("generate_version_file")` uses `script = "$ark_root/gn/build/make_version_file.sh"`
- `static_core/libarkbase/BUILD.gn` - Same version file generation
- `static_vm_config.gni` - `build_root = "$ark_root/gn/build"` for script path resolution

## References

- Verified: No BUILD.gn in this directory - script-only module
- Verified: Scripts are referenced from other BUILD.gn files via `script = "$ark_root/gn/build/..."`
- Verified: Version file invoked as `make_version_file.sh <git_root_dir> <input_file> <output_file>`
