# Install dependencies:

Ensure the following components are installed:
- repo
- clang++-14
- clang-format-14
- clang-tidy-14
- doxygen
- graphviz
- Git LFS

# How to download

```sh
repo init -u https://gitee.com/ark-standalone-build/manifest.git -b master
repo sync -c -j8
repo forall -c 'git lfs pull'
./prebuilts_download.sh
```

# How to build and test

## Build AbcKit for Linux

```sh
# Debug mode
./ark.py x64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
# Release mode
./ark.py x64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
```

## Build AbcKit for Windows
```sh
# Debug mode
./ark.py mingw_x86_64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
# Release mode
./ark.py mingw_x86_64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
```

## Output Locations

The generated abckit binary and libabckit.so/libabckit.dll are in the `out/${target}/arkcompiler/runtime_core/libabckit` directory.

The generated binary and library depend on the libraries in:
- `out/${target}/arkcompiler/runtime_core/`
- `out/${target}/arkcompiler/ets_runtime/`
- `out/${target}/thirdparty/icu/`
- `out/${target}/thirdparty/zlib/`

NOTE: Replace ${target} with build target: x64.debug, x64.release, mingw\_x86\_64.debug or mingw\_x86\_64.release.

## Run unit tests

```sh
# Debug mode
./ark.py x64.debug abckit_tests --gn-args="is_standard_system=true abckit_enable=true"
# Release mode
./ark.py x64.release abckit_tests --gn-args="is_standard_system=true abckit_enable=true"
```

## Run unit tests with sanitizers

```sh
# Debug mode
./ark.py x64.debug abckit_tests --gn-args="is_standard_system=true abckit_enable=true libabckit_with_sanitizers=true"
# Release mode
./ark.py x64.release abckit_tests --gn-args="is_standard_system=true abckit_enable=true libabckit_with_sanitizers=true"
```

# How to use AbcKit

Requirements:
- AbcKit works with the release version of abc files
- Currently AbcKit supports abc files compiled from ArkTS and JS
- Currently AbcKit does not support CommonJS modules

AbcKit provides C API and C++ API to inspect and modify abc file.

Users can use AbcKit API to implement abckit plugins.

There are two ways to use abckit: executable binary and dynamic library.

(1) executable binary

Implement the following function:

```cpp
extern "C" int Entry(AbckitFile *file)
{
    // Users can use AbcKit API here.
    return 0;
}
```

Compile the code to a dynamic library (e.g. transformer.so) as an abckit plugin.

Run the following command to execute the abckit plugin:

```sh
./abckit --plugin-path transformer.so --input-file /path/to/input.abc --output-file /path/to/output.abc
```

(2) dynamic library

Users can load libabckit.so/libabckit.dll then use AbcKit API.

# Full tests (`self-check.sh`)

Remove out, build AbcKit, execute format, tidy, unit-tests and stress tests in debug and release modes:

```sh
./arkcompiler/runtime_core/libabckit/scripts/self-check.sh --dir=/path/to/standalone/root
```

# Code coverage (`self-check.sh`)

Remove out, build AbcKit, execute unit-tests and stress tests in debug mode and collect code coverage:

```sh
./arkcompiler/runtime_core/libabckit/scripts/self-check.sh --dir=/path/to/standalone/root --coverage
```
