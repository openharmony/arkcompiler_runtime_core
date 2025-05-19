## Prerequisites

PANDA and VMB are built and installed per [vm-benchmarks/interop.readme.md](../../interop.readme.md).

The following conventions must be followed:
- Bench Unit directories must follow naming convention depending on interop mode: `bu_a2a`, `bu_a2j`, `bu_j2a`, `bu_j2j`.

- Bench Unit doclets for respective mode must have tags with same names as of its directory.

- In `bu_a2j` mode benchmark methods must have name `test()`.

If needed, see also [vm-benchmarks/readme.md](../../readme.md) for more details on how to use and configure VMB benchmarks.

## Sanity check run

Note `-wi=1 -mi=1` parameters below mean minimal number of iterations to execute and is intended only as a smoke test verifying that code can run. For realistic benchmarks, customise these values or remove them to use defaults.

```
cd $PANDA_ROOT/tests/vm-benchmarks
# assume PANDA and VMB are built and installed per interop.readme.md
export PANDA_BUILD=$PANDA_ROOT/build
export PANDA_STDLIB_SRC=$PANDA_ROOT/plugins/ets/stdlib
vmb all -p arkts_node_interop_host --aot-skip-libs -v debug \
  -wi=1 -mi=1 \
  --ark-custom-option=--gc-trigger-type=heap-trigger \
  --ark-custom-option=--compiler-enable-jit=true \
  --ark-custom-option=--run-gc-in-place=false \
  --ark-custom-option=--log-components=ets_interop_js \
  --ark-custom-option=--load-runtimes=ets \
  --exclude-list $PANDA_ROOT/tests/vm-benchmarks/examples/benchmarks-interop-templates/exclude-interop-benchmarks-examples.txt 
  $PANDA_ROOT/tests/vm-benchmarks/examples/benchmarks-interop-templates
```

## Example results output

```
4 tests; 0 failed; 0 excluded; Time(GM): 1318.6 Size(GM): 148336.7 RSS(GM): 98233.2
===================================================================================


Test           |   Time   | CodeSize |   RSS    | Status  |
===========================================================
BenchA2a_test | 4.02e-01 | 1.52e+05 | 5.97e+04 | Passed  |
BenchA2j_test | 4.92e+08 | 1.52e+05 | 1.14e+05 | Passed  |
benchJ2a_test | 2.86e+04 | 1.46e+05 | 4.52e+05 | Passed  |
benchJ2j_test | 4.58e-01 | 0.00e+00 | 4.71e+04 | Passed  |
```

# How to run for ArkjsVM

Along with [Prerequisites](#Prerequisites) above, ArkjsVM is built and installed per [tests/interop_js/README.MD](../../../../plugins/ets/tests/interop_js/README.MD)

```
cd $PANDA_ROOT/tests/vm-benchmarks
# assume ArkjsVM built and installed per $PANDA_ROOT/plugins/ets/tests/interop_js/README.MD
# assume PANDA and VMB are built and installed per $PANDA_ROOT/tests/vm-benchmarks/interop.readme.md
export PANDA_BUILD=$PANDA_ROOT/build
export PANDA_STDLIB_SRC=$PANDA_ROOT/plugins/ets/stdlib
vmb all -p arkts_arkjs_interop_host --aot-skip-libs -v debug \
  -wi=1 -mi=1 \
  --ark-custom-option=--gc-trigger-type=heap-trigger \
  --ark-custom-option=--compiler-enable-jit=true \
  --ark-custom-option=--run-gc-in-place=false \
  --ark-custom-option=--log-components=ets_interop_js \
  --ark-custom-option=--load-runtimes=ets \
  --exclude-list $PANDA_ROOT/tests/vm-benchmarks/examples/benchmarks-interop-templates/exclude-interop-benchmarks-examples.txt 
  $PANDA_ROOT/tests/vm-benchmarks/examples/benchmarks-interop-templates
```
