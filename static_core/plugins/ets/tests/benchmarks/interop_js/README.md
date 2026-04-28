# Interoperability Benchmarks

## Tests

There are 4 type of tests:

- Dynamic-To-Static (tagged `d2s`, platform `interop_d2s`)
- Static-To-Dynamic (tagged `s2d`, platform `interop_s2d`)
- Dynamic-To-Dynamic (tagged `d2d`, platform `interop_d2d`)
- Static-To-Static (tagged `s2s`, platform `arkts_host` i.e. `ArkTS` + `ets` import )

First letter signifies context in which measurement cycle runs,
calling methods from context signified by second letter.

Each type could only be run in separate launch of `vmb`.
Dynamic runs are performed using binaries from `ark.py` hybrid build.
However, `s2s` mode requires usual `cmake` build from `static_core`.

Note: `lib*` prefix is reserved for 'imported' code,
this should not be mixed with 'included' or 'benchmark' files.
All other file and directory names are arbitrary.

## How to run

```sh
# Build vmb if needed
pushd $PANDA_ROOT/arkcompiler/runtime_core/static_core/tests/vm-benchmarks
make vmb
popd

# Assuming ArkJSVM and ArkVM were built by `ark.py` with interop support
export PANDA_BUILD=$PANDA_ROOT/out/x64.release/

# This is just a short cut and has no special meaning.
# Free arguments to `vmb` could be any set of paths (files and/or directories)
TESTS=$PANDA_ROOT/arkcompiler/runtime_core/static_core/plugins/ets/tests/benchmarks/interop_js

# Run Dynamic-To-Static benches:
vmb all -p interop_d2s -T d2s -L js -l js --exclude-list=$TESTS/exclude-interop-d2s.txt --timeout=60 $TESTS

# Run Static-To-Dynamic benches:
vmb all -p interop_s2d -T s2d -A --exclude-list=$TESTS/exclude-interop-s2d.txt --timeout=60 $TESTS

# Run Dynamic-To-Dynamic benches:
vmb all -p interop_d2d -T d2d -L js -l js --exclude-list=$TESTS/exclude-interop-d2d.txt --timeout=60 $TESTS

# Run Static-To-Static benches, `cmake` build required:
export PANDA_BUILD=$PANDA_ROOT/arkcompiler/runtime_core/static_core/build/
vmb all -p arkts_host -T s2s -A --exclude-list=$TESTS/exclude-interop-s2s.txt --timeout=60 $TESTS
```

## Other options

See also [vmb manual](../../../../../tests/vm-benchmarks/readme.md)

```sh
# List all options
vmb help

#  List command-specific options: vmb [gen,run,report,all,version,list] --help
vmb all --help
vmb report --help
```

```sh
# Example
vmb all -p interop_s2d \
  --fast-iters=1 \
  --aot-skip-libs \
  -v debug \
  --tests=testToString_hex \
  --tags=s2d \
  --skip-tags=promise \
  --arkjs_interop-custom-option=--gc-trigger-type=heap-trigger \
  --arkjs_interop-custom-option=--compiler-enable-jit=true \
  --arkjs_interop-custom-option=--run-gc-in-place=false \
  --arkjs_interop-custom-option=--log-components=ets_interop_js \
  --arkjs_interop-custom-option=--load-runtimes=ets \
  --exclude-list=$TESTS/exclude-interop-s2d.txt \
  $TESTS
```

## How to compare results

```sh
# run 1:
 vmb all -p interop_s2d -T s2d -A --report-json=1.json $TESTS/array_map_reduce/

# run 2:
 vmb all -p interop_d2s -T d2s -L js -l js --report-json=2.json $TESTS/array_map_reduce/

# compare
vmb report --compare --full 1.json 2.json

## print json report
vmb report --full 1.json
```
