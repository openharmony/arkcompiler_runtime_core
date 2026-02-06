---
name: build-stdlib
description: Build the ArkTS stdlib project using CMake. Use when compiling the stdlib, building project from ../../../, running cmake configuration, or compiling with ninja. Handles CMake configuration from project root, Ninja compilation, build directory management, and clean rebuilds.
---

# Build Stdlib

Build the ArkTS stdlib project using CMake and Ninja.

## Project Structure

- **Project root**: `../../../` (static_core)
- **Build directory**: `../../../build/`
- **Source location**: Current directory is `runtime_core/static_core/plugins/ets/stdlib/`

## Quick Build

From current directory (stdlib/):

```bash
# Configure (if needed)
cd ../../../ 
cmake -B build -DCMAKE_BUILD_TYPE=Debug -GNinja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake \
    -S . -Werror=dev -DCMAKE_CXX_FLAGS="-fno-limit-debug-info" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPANDA_ETS_INTEROP_JS=ON -DPANDA_WITH_TESTS=ON

# Build
cd build && ninja panda_bins

# Or single command from root:
cmake --build build --target panda_bins
```

## Build Workflow

### Initial Configuration

First time setup from project root:

```bash
cd ../../../
cmake -B build -DCMAKE_BUILD_TYPE=Debug -GNinja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake -S . -Werror=dev -DCMAKE_CXX_FLAGS="-fno-limit-debug-info" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPANDA_ETS_INTEROP_JS=ON -DPANDA_WITH_TESTS=ON

Common build types:
- `Release` - Optimized production build
- `Debug` - Debug symbols, no optimization
- `RelWithDebInfo` - Optimized with debug symbols
- `FastVerify` - Custom fast verification build

### Compilation

After configuration:

```bash
# From project root
cmake --build build --target panda_bins

# Or directly in build directory
cd build && ninja panda_bins

# Parallel build (default)
cd build && ninja -j$(nproc) panda_bins

# Specific target
cd build && ninja panda_bins
```

### Clean Rebuild

```bash
# From project root
cd ../../../
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Debug -GNinja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake \
    -S . -Werror=dev -DCMAKE_CXX_FLAGS="-fno-limit-debug-info" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPANDA_ETS_INTEROP_JS=ON \
    -DPANDA_WITH_TESTS=ON
cmake --build build --target panda_bins
```

## Build Targets

Key targets related to stdlib:

- `arkstdlib` - Main stdlib target
- `copy_stdlib` - Copy and process stdlib files
- `plugins/ets/stdlib` - ETS stdlib plugin
- `panda_bins` - Compile binaries for panda normal execution

Build specific target:

```bash
cd ../../../build && ninja panda_bins
```

## CMake Options

Common configuration options:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
    -GNinja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake \
    -S . -Werror=dev -DCMAKE_CXX_FLAGS="-fno-limit-debug-info" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPANDA_ETS_INTEROP_JS=ON -DPANDA_WITH_TESTS=ON
```

## Build Outputs

After successful build:

- Libraries in `../../../build/lib/`
- Executables in `../../../build/bin/`
- Stdlib outputs in `../../../build/plugins/ets/stdlib/`
- Stdlib abc file in `../../../build/plugins/ets/etsstdlib.abc`

## Troubleshooting

### CMake Configuration Fails

```bash
# Clean cache and reconfigure
rm -rf build/CMakeCache.txt build/CMakeFiles
cd ../../../ && cmake -B build -DCMAKE_BUILD_TYPE=Debug -GNinja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake -S . -Werror=dev -DCMAKE_CXX_FLAGS="-fno-limit-debug-info" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPANDA_ETS_INTEROP_JS=ON -DPANDA_WITH_TESTS=ON
```

### Ninja Build Fails

```bash
# Clean specific target and rebuild
cd ../../../build && ninja -t clean panda_bins && ninja panda_bins
```

### Missing Dependencies

Check CMake error messages for required packages. Common issues:
- Missing compiler (clang/gcc)
- Missing third-party libraries
- Incorrect C++ compiler version

## Verification

Check build success:

```bash
# Check library exists
cd ../../../ # runtime_core/static_core
ls -lh build/plugins/ets/etsstdlib.abc

# Check stdlib outputs
ls -lh build/plugins/ets/stdlib/*
```
