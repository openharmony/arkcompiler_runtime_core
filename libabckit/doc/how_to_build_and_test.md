# How to download

```sh
repo init -u https://gitee.com/ark-standalone-build/manifest.git -b master
repo sync -c -j8
repo forall -c 'git lfs pull'
./prebuilts_download.sh
```

# How to build and test

Build AbcKit:

```sh
# Debug mode
./ark.py x64.debug libabckit_packages --gn-args="is_standard_system=true enable_libabckit=true"
# Release mode
./ark.py x64.release libabckit_packages --gn-args="is_standard_system=true enable_libabckit=true"
```

Run unit tests:

```sh
# Debug mode
./ark.py x64.debug libabckit_tests --gn-args="is_standard_system=true enable_libabckit=true"
# Release mode
./ark.py x64.release libabckit_tests --gn-args="is_standard_system=true enable_libabckit=true"
```

Run unit tests with sanitizers:

```sh
# Debug mode
./ark.py x64.debug libabckit_tests --gn-args="is_standard_system=true enable_libabckit=true libabckit_with_sanitizers=true"
# Release mode
./ark.py x64.release libabckit_tests --gn-args="is_standard_system=true enable_libabckit=true libabckit_with_sanitizers=true"
```

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
