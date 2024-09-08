# Bootstrapping

Currently an officially supported host OS for development, building and testing is Ubuntu 18.04 and Ubuntu 20.04.
The full list of packages required for building and testing the project is specified in
`scripts/deps-lists` files. These packages can be installed either manually or by running
a bootstrap script:

```
$ sudo ./scripts/install-deps-ubuntu
```

For more bootstrapping options, run:

```
$ sudo ./scripts/install-deps-ubuntu --help
```

E.g. for development purposes you should run:

```
$ sudo ./scripts/install-deps-ubuntu -i=dev
```

If you want additionally to install Python dependencies for running tests add a parameter `-i=test`:

```
$ sudo ./scripts/install-deps-ubuntu -i=dev -i=test
```
It creates a virtual environment .venv-panda in your home directory with all required dependencies. 
Later, tests Python scripts can activate this environment. If you already have run with the parameter `-i=test`
the second time it might be skipped.

# Third party

Panda uses third party libraries. To install the libraries and apply patches, run:

```
$ ./scripts/install-third-party --force-clone
```

# Building

Assuming your system is bootstrapped, please run the following commands in the root of the project:

```
$ mkdir panda-build
$ cd panda-build
$ cmake /path/to/panda/repository -GNinja
$ ninja
```

This will build Panda in debug mode with your default C++ compiler. All supported compilers can be
found in `cmake/toolchain`. E.g., to build with Clang 14, pass a path to the corresponding toolchain
file during configuration:

```
$ cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/panda/repository/cmake/toolchain/host_clang_14.cmake /path/to/panda/repository
```

## Explicitly setting build types

Recommended way to build Panda is to set `CMAKE_BUILD_TYPE` variable explicitly during configurations. Supported values are:

| Mode         | Assertions | Optimizations         | Debug info           |
| ----         | ---------- | -------------         | ----------           |
| `Debug `     | Yes        | None (CMake default)  | `-g` (CMake default) |
| `Release`    | No         | `-O3` (CMake default) | None (CMake default) |
| `FastVerify` | Yes        | `-O2`                 | `-ggdb3`             |

Notes:

* Other common modes (`RelWithDebInfo`, `MinSizeRel`, `DebugDetailed`) should work but they are not tested in CI.
* Unlike `RelWithDebInfo`, `FastVerify` preserves assertions (and provides more verbose debug information).
  Use this build type for running heavy test suites when you want both fast-running code and debuggability.
* `DebugDetailed` gives more debug information than `Debug`, it can be usefull for debugging unit tests for example.

Example:

```
$ cmake -DCMAKE_BUILD_TYPE=Release ...
```

## Building with LLVM Backend

If you want to build Ark with LLVM Backend you need to build first special modified LLVM 15 binaries.
  * Clone repository from [gitee.com](https://gitee.com/openharmony/third_party_llvm-project)
  * Check [README](scripts/llvm/README.md) file about build process
  * Use build [script](scripts/llvm/build_llvm.sh)

If modified LLVM available in `/opt`, the following two options are necessary
to build Ark with LLVM Backend functions.

```cmake
cmake -DPANDA_LLVM_BACKEND=true -DLLVM_TARGET_PATH=/opt/llvm-15-{type}-{arch} ...
```

The `PANDA_LLVM_BACKEND` enables:

1. LLVM Irtoc Interpreter. Use `-DPANDA_LLVM_INTERPRETER=OFF` to disable.
2. LLVM Fastpaths compilation. Use `-DPANDA_LLVM_FASTPATH=OFF` to disable.
3. LLVM Interpreter inlining. Use `-DPANDA_LLVM_INTERPRETER_INLINING=OFF` to disable.
4. LLVM AOT compiler. Use `-DPANDA_LLVM_AOT=OFF` to disable.

`PANDA_LLVM_INTERPRETER`, `PANDA_LLVM_FASTPATH`, and `PANDA_LLVM_AOT` are `ON` if `PANDA_LLVM_BACKEND` is turned on.

It is recommended to choose `clang` compiler using toolchain files: `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake`.
By default GNU compiler `c++` is used, but some features are not available in such `gcc` builds.

## Cross ARM64 build with LLVM Backend

For cross-build, when e.g. `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/cross-clang-14-qemu-aarch64.cmake` is used,
two LLVM-paths options are required when all LLVM Backend functions are enabled.
  * First one is target LLVM, like `-DLLVM_TARGET_PATH=/opt/llvm-15-debug-aarch64`
    * It is required for AOT, so, alternatively, you can use `-DPANDA_LLVM_AOT=OFF`.
  * Second one is host LLVM, like `-DLLVM_HOST_PATH=/opt/llvm-15-debug-x86_64`
    * It is required for Irtoc compilation, so, alternatively, you can disable appropriate Interpreter and FastPath options (see above).

## Running tests with QEMU for cross-compiled aarch64/arm builds

Recommended QEMU version for running tests is 6.2.0 (but 5.1+ should be ok, too). By default, it is downloaded and installed during environment bootstrap. Any system-installed package is left intact. If recommended QEMU version is not accessible via $PATH it can be specified during configuration time:

```
# If QEMU is available as /opt/qemu-6.2.0/bin/qemu-aarch64
$ cmake -DQEMU_PREFIX=/opt/qemu-6.2.0 ...
```

## Building with GN

### Using bootstrap

Build `arkts_bin`, `ark_aot`, `es2panda`, `verifier_bin` and `ets_interop_js_napi` targets.
```
./scripts/build-panda-with-gn
```

### Mannually

1. Getting GN binary

```
$ git clone https://gn.googlesource.com/gn
$ cd gn
$ python build/gen.py
$ ninja -C out
```

2. Build panda using gn (`arkts_asm`, `arkts_disasm`, `ark_aot`, `ark_aotdump`, `arkts_bin`, `es2panda`, and `verifier_bin` targets are supported)

```
$ cd /path/to/panda/repository
$ /path/to/gn/repository/out/gn gen out
$ ninja -C out arkts_bin
```
When standard system, use
```
$ cd /path/to/panda/repository
$ /path/to/gn/repository/out/gn --args=is_standard_system=true gen out
$ ninja -C out <target name>
```
To use LLVM Backend add the following arguments:
```
$ /path/to/gn/repository/out/gn out is_llvmbackend=true llvm_target_dir="/opt/llvm-15-{type}-{arch}" ...
```
Setting `is_llvmbackend=true` option enables the same scenarios as `-DPANDA_LLVM_BACKEND=true` option in cmake builds

## Further reading

* For more details, please see [build system readme](cmake/README.md).

# Testing

For testing, the following umbrella targets that guarantee building prior to running may be used:

* `tests`, for running all testing suites.
* `tests_full`, for running all testing suites and various code linters.

# Clang tools

`clang-format` and `clang-tidy` checks are integrated into build system and can be called by target of build system:
```
$ ninja code-style-check # clang-format
$ ninja clang-tidy-check # clang-tidy
```

# Test coverage

In order to measure bytecode optimizer coverage, configure your build with -DENABLE_BYTECODE_OPTIMIZER_COVERAGE=true. Then run:
```
$ ninja bytecode_optimizer_coverage
```

# Benchmarking

To build and run benchmarks, please use the umbrella target `benchmarks` or any of its
dependencies. Please see the root `CMakeLists.txt` for more details.

NB! Make sure that you configure your build with `-DCMAKE_BUILD_TYPE=Release`, otherwise
your run will most likely be dead slow.

# Running

## Running assembler

Assuming that you are in `panda-build` directory, please run:

```
$ ./bin/ark_asm /path/to/panda-assembly.pa /path/to/binary/output.abc
```

## Running interpreter

Assuming that your main function is defined as `.function main(...)` in the assembly listing,
and `/path/to/binary/output.abc` is the result of the assembler, please run:

```
$ ./bin/ark /path/to/binary/output.abc _GLOBAL::main
```
