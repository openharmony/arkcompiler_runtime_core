# AbcKit

AbcKit is a library for `abc` file inspection and modification.

Documentation:

- [How to build and test](doc/how_to_build_and_test.md)
- [Cook Book](doc/mini_cookbook.md)
- [Implementation description](doc/implementation_description.md)

## How to download and build

### Download:

```sh
repo init -u https://gitee.com/ark-standalone-build/manifest.git -b master
repo sync -c -j8
repo forall -c 'git lfs pull'
./prebuilts_download.sh
```

### Build AbcKit:

```sh
./ark.py x64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
```

### Generate doxygen documentation
```sh
cd out/x64.release
ninja abckit_documentation
```

See generated documentation in `out/x64.release/gen/arkcompiler/runtime_core/libabckit/doxygen`