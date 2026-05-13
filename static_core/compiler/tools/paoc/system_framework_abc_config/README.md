# system_framework_abc_config

This directory is reserved for **method config files** used by `host_aot_compile.py`.

## config file format (strict)

Each config file must follow this strict format:

1. **Line 1**: target abc path on device (absolute path style), for example:
   - `/system/framework/xxx.abc`
2. **Line 2**: full method names with signatures to be compiled

The config must use method names with signatures. This is required to avoid
ambiguous matching when different functions share the same name.

### Specification

1. Each framework ABC needs its own config file. config files for different framework ABCs do not affect each other.
2. Functions with the same name in different classes are distinguished by their signatures and do not affect each other.
3. One config file can contain multiple functions for the same framework ABC.

### Example

```text
/system/framework/test.abc
void test.ClassA::Method(f64)
i32 test.ClassB::Method(i32)
```

## Local test

Run the local integration test with:

```bash
cd static_core
cmake --build build --target compiler_host_aot_test -j$(nproc)
```
