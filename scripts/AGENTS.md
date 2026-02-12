# Scripts Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/scripts
> Scope: scripts/

## Purpose

Provides **scripts and data** for **build, environment, format checks, and debugging/analysis**, including dependency lists, install scripts, code format checks, and debug helpers. This directory **does not define GN or CMake targets**; it is only used by humans, CI, or other scripts/BUILD.gn via path reference.

## Where to Edit

### Module Entry Points

- `scripts/dep-lists/` - Dependency lists for install/CI
- `scripts/third-party-lists/` - Third-party library lists
- `scripts/install-deps-*` - Dependency install scripts
- `scripts/run-clang-format` - Code format invocation
- `scripts/run-check-concurrency-format.sh` - Concurrency format check
- `scripts/run_check_atomic_format.py` - Atomic operation format check
- `scripts/gc_pause_stats.py` - GC pause stats
- `scripts/memdump.py` - Memory dump analysis
- `scripts/memusage.py` - Memory usage analysis
- `scripts/trace_enable.sh` - Trace toggle

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not break script paths, CLI, or output format already used by BUILD.gn or CI
- ⚠️ Do not add BUILD.gn or define targets in this directory; it is script and data only

## Validation Checklist

### Module Dependencies

**Key upstream dependencies**:
- Environment - Shell scripts need bash; Python scripts need Python

**Key downstream dependents**:
- **static_core/BUILD.gn** - Uses `script = "${ark_root}/scripts/code_style/code_style_check.py"`
- **static_core/libllvmbackend/BUILD.gn** - Uses `script = "$ark_root/scripts/llvm/llvm-config.sh"`
- **Humans/CI** - Runs dep-lists, install-deps-*, run-clang-format, etc.

## References

- Verified: No BUILD.gn - scripts are run by humans, CI, or other modules' actions
- Verified: Multi-purpose - environment setup, code style, runtime analysis
- Verified: Cross-directory use - static_core may reference scripts from different paths