---
name: urunner-stdlib
description: Run ets_func_tests using the Universal Runner (urunner) tool. Use when executing stdlib tests, running test suites, or testing ETS functionality. Handles running tests with runner.sh from ../../../tests/tests-u-runner-2/, selecting workflows and test suites, configuring test options, and running specific tests or full test suites.
---

# Urunner Stdlib

Run ets_func_tests using the Universal Runner (urunner) tool.

## Tool Location

- **Runner directory**: `../../../tests/tests-u-runner-2/`
- **Runner script**: `runner.sh` or `main.py`
- **Configuration**: `cfg/workflows/` and `cfg/test-suites/`

## Prerequisites

1. **Panda build** - Build must be completed (see build-stdlib skill)
2. **Python 3.8+** - 3.10 recommended
3. **Virtual environment** - Created via `sudo static_core/scripts/install-deps-ubuntu -i=test`
4. **Environment setup** - `~/.urunner.env` must exist with required variables

### Environment Variables

Create `~/.urunner.env`:

```bash
ARKCOMPILER_RUNTIME_CORE_PATH=<path to arkcompiler_runtime_core>
ARKCOMPILER_ETS_FRONTEND_PATH=<path to arkcompiler_ets_frontend>
PANDA_BUILD=<path to build folder>
WORK_DIR=<path to temporary working folder>
```

Or run: `./runner.sh init` for interactive setup.

## Quick Start

From stdlib directory (stdlib/):

Always use `--filter` option to filter affected api, to check which tests are affected need to check `../tests/ets_func_tests/std/`. 
Example: `--filter std/core/json` if changed json api. If changed `Intl` need to use `--filter/std/core/Intl*`.

```bash
# Run only affected tests  std/core/json should be changed to affected tests like regex array etc...
cd ../../../tests/tests-u-runner-2/
./runner.sh panda-int ets-func-tests --show-progress --force-generate --processes=all --filter std/core/json`
```

```bash
# Run all stdlib tests
cd ../../../tests/tests-u-runner-2/
./runner.sh panda-int ets-func-tests ets-es-checked --show-progress --force-generate --processes=all 

# Or with specific options
./runner.sh panda-int ets-func-tests --show-progress --force-generate
```

## Basic Usage

### Standard Test Run

```bash
cd ../../../tests/tests-u-runner-2/
./runner.sh <workflow> <test-suite> [options]
```

### Common Workflows

From `cfg/workflows/`:
- `panda-int` - Panda interpreter (most common for stdlib)
- `panda-aot` - Panda AOT compilation
- `ark` - ARK runtime
- `checker` - Type checker only

### Test Suite for Stdlib

The stdlib tests are typically run with:
- Test suite: `ets-func-tests` (may vary, check `cfg/test-suites/`)

## Running Specific Tests

### Filter Tests

```bash
# Run specific test file
./runner.sh panda-int ets-func-tests --test-file StringCharAtTest.ets

# Filter by pattern
./runner.sh panda-int ets-func-tests --filter "String*"

# Multiple filters
./runner.sh panda-int ets-func-tests --filter "core/*" --filter "containers/*"
```

### Test Directory

```bash
# Run tests in specific subdirectory
./runner.sh panda-int ets-func-tests --test-dir core/
```

## Common Options

```bash
--show-progress          # Show progress bar
--processes <N>          # Number of parallel processes (default: all)
--force-generate         # Force regeneration of test files
--extension ets          # File extension (default: ets)
--load-runtimes ets      # Runtimes to load
--opt-level 2            # Optimization level (0-3)
--timeout <N>            # Test timeout in seconds
```

## Examples

### Run All Stdlib Tests

```bash
cd ../../../tests/tests-u-runner-2/
./runner.sh panda-int ets-func-tests --show-progress
```

### Run Specific Test File

```bash
./runner.sh panda-int ets-func-tests --test-file StringGetTest.ets
```

### Run With Custom Options

```bash
./runner.sh panda-int ets-func-tests \
    --show-progress \
    --processes 4 \
    --force-generate
```

### Run Tests Matching Pattern

```bash
./runner.sh panda-int ets-func-tests --filter "String*"
```

## Output and Results

- **Reports**: Generated in `$WORK_DIR/reports/`
- **Logs**: Available in work directory
- **Exit codes**: 0 = success, non-zero = failures

## Troubleshooting

### Environment Not Set Up

```bash
# Create .urunner.env
./runner.sh init

# Or manually create ~/.urunner.env with required variables
```

### Virtual Environment Missing

```bash
sudo static_core/scripts/install-deps-ubuntu -i=test
```

### Build Not Found

```bash
# Ensure panda is built
cd ../../../
cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake \
    -S . -Werror=dev  -DPANDA_WITH_TESTS=ON\
    -DPANDA_ETS_INTEROP_JS=ON
cmake --build build --target panda_bins etssdk
```

## Direct Python Usage

If preferred, run `main.py` directly:

```bash
cd ../../../tests/tests-u-runner-2/
source ~/.venv-panda/bin/activate
python3 main.py panda-int ets-func-tests --show-progress
deactivate
```
