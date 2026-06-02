# Build and Test Knowledge

This document records only the build and test commands and constraints.

## Core Model

```
Source (.ets) → CMake/GN build → abc file → test verification
```

## Build Systems

| System | Purpose | Execution Directory |
| --- | --- | --- |
| CMake | Primary development build | `static_core/` (i.e., `../../../`) |
| GN | OpenHarmony integration | Project root directory |

### CMake Build (Primary)

```bash
# Execute from the static_core/ directory (../../../)
cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake \
    -S . -Werror=dev -DPANDA_WITH_TESTS=ON -DPANDA_ETS_INTEROP_JS=ON
cmake --build build --target panda_bins etssdk
```

### GN Build

```bash
gn gen out/default
ninja -C out/default
# Key target: copy_stdlib
```

### Python Packaging

```bash
python3 package_stdlib.py --std-dir std --target-dir out/arkcompiler/stdlib --json-file hiddable_APIs.json
```

- `package_stdlib.py` filters APIs based on `hiddable_APIs.json` (removes `export` from specific interfaces)
- `hiddable_APIs.json` is provided by the build system and is **not** tracked in the stdlib directory

## Testing

### ArkTest Framework

Defined in `std/testing/`. Usage:

```typescript
function main(): int {
    let myTestsuite = new arktest.ArkTestsuite("myTestsuite");
    myTestsuite.addTest("TestName", () => {
        arktest.assertEQ(actual, expected);
    });
    return myTestsuite.run();
}
```

### Universal Runner (urunner)

```bash
# Run ets_func_tests (execute from static_core/)
PANDA_BUILD="./build" ARKCOMPILER_RUNTIME_CORE_PATH=".." ARKCOMPILER_ETS_FRONTEND_PATH="../../ets_frontend" \
  WORK_DIR="/tmp/ets" ./tests/tests-u-runner-2/runner.sh \
  panda-int ets-func-tests --show-progress --force-generate --processes=all --filter '**/std/core/json'

# Run checker tests (execute from static_core/)
./tests/tests-u-runner-2/runner.sh checker compiler_checker_suite --show-progress --force-generate --processes=all
```

## Constraints

- After modifying .ets files, you **must** rebuild before running tests — reason: old bytecode does not reflect source changes
- `main()` **must** return `myTestsuite.run()`; returning a constant is **prohibited** — reason: returning a constant causes tests to always pass/fail, masking the real results
- urunner **must** use `--filter` to scope to affected APIs — reason: without a filter, all tests are run, wasting significant time
- Test filter format: `--filter **/std/core/<api_name>`; for example, for `Intl` use `--filter **/std/core/Intl*`
- Reference directory for affected tests: `../tests/ets_func_tests/std/`

## Common Issues

| Issue | Cause | Solution |
| --- | --- | --- |
| Import error | Incorrect path mapping | Check `stdconfig.json`; use full paths such as `std.core.Array` |
| Incorrect test return value | main() returns a constant | Return `myTestsuite.run()` |
| Missing hiddable_APIs.json | Build system has not generated it | This file is provided by the build system and is not tracked in stdlib |

## Pre-Modification Checklist

- Have you rebuilt after making changes?
- Does main() return the testsuite result instead of a constant?
- Is urunner using the correct `--filter`?

## Code and Tests

- ArkTest framework: `std/testing/`
- ets_func_tests: `../tests/ets_func_tests/std/`
- urunner: `../../../tests/tests-u-runner-2/`
- Build tools: `SKILLS/build-stdlib/`
- Test runner: `SKILLS/urunner-stdlib/`
- Test generation: `SKILLS/arktest-generator/`
