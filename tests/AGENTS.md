# Tests Module AGENTS.md

> Verified on commit: f965a250e9a4ad664a282f7e220a42a744964a5a
> Last updated: 2026-02-13
> Assumed workspace: arkcompiler/runtime_core/tests
> Scope: tests/

## Purpose

Provides **test infrastructure** (e.g. `host_unittest_action` template, `test_helper.gni`), **Fuzz test suite** (multiple fuzzer targets), and **test data and scripts** (benchmarks, checked, cts-assembly, etc.). This directory does not produce runtime libraries or tools; it only supports building and running unit and Fuzz tests.

## Quick Start

### Build Commands

```bash
# Build all Fuzz targets (if root BUILD.gn or upper layer includes tests/fuzztest:fuzztest)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/tests/fuzztest:fuzztest

# Build a single fuzzer example
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core/tests/fuzztest/openpandafile_fuzzer:fuzztest

# Run host unit tests (module tests that use this directory's test_helper)
./build.sh --product-name <product_name> --build-target arkcompiler/runtime_core:runtime_core_host_unittest
```

## Where to Edit

### Module Entry Points

- `tests/test_helper.gni` - Test helpers: host_unittest_action template
- `tests/fuzztest/BUILD.gn` - Fuzz test group and per-fuzzer targets
- `tests/fuzztest/*_fuzzer/` - Individual fuzzer directories

### Module-Specific "Do Not Edit" Boundaries

- ⚠️ Do not break the `host_unittest_action` interface (parameters, script, args), or existing module tests/BUILD.gn will fail
- ⚠️ Do not narrow the fuzzed API or disable sanitizers just to "pass Fuzz" unless there is a clear need

## Validation Checklist

### Module-Specific Test Commands

- **Fuzz tests**: Each `*_fuzzer` target is a Fuzz executable; run with libFuzzer/AFL etc.
- **Unit tests**: This directory has no dedicated "tests" unit-test target; unit tests live in each module's `*/tests:host_unittest`

### Module Dependencies

**Key upstream dependencies**:
- `test_helper.gni` - Depends on `ark_root.gni`, `$build_root/test.gni`
- Each fuzzer - Depends on library under test (e.g. libarkbase_static, libarkfile_static) and Fuzz runtime

**Key downstream dependents**:
- **Module tests** - Use `host_unittest_action` template via `import("$ark_root/tests/test_helper.gni")`
- **Root BUILD.gn** - May depend on `tests/fuzztest:fuzztest` to aggregate Fuzz targets

## References

- Verified: test_helper.gni defines host_unittest_action template imported by module tests
- Verified: fuzztest defines 30+ fuzzer targets covering libpandabase, libpandafile, etc.
- Verified: benchmarks/, checked/, verifier-tests/, etc. are data and script directories with no top-level GN targets