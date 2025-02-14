## Prerequisites

PANDA and VMB are built and installed per [vm-benchmarks/interop.readme.md](../../interop.readme.md).

The following conventions must be followed:
- Bench Unit directories must follow naming convention depending on interop mode: `bu_a2a`, `bu_a2j`, `bu_j2a`, `bu_j2j`.

- Bench Unit doclets for respective mode must have tags with same names as of its directory.

- In `bu_a2j` mode benchmark methods must have name `test()`.

If needed, see also [vm-benchmarks/readme.md](../../readme.md) for more details on how to use and configure VMB benchmarks.

## Sanity check run

```
cd $PANDA_ROOT/tests/vm-benchmarks
# assume PANDA and VMB are built and installed per interop.readme.md
export PANDA_BUILD=$PANDA_ROOT/build
export PANDA_STDLIB_SRC=$PANDA_ROOT/plugins/ets/stdlib
vmb all -p arkts_node_interop_host --aot-skip-libs -v debug $PANDA_ROOT/tests/vm-benchmarks/examples/benchmarks-interop-templates
```

## Example results output

```
4 tests; 0 failed; 0 excluded; Time(GM): 1318.6 Size(GM): 148336.7 RSS(GM): 98233.2
===================================================================================


Test           |   Time   | CodeSize |   RSS    | Status  |
===========================================================
BenchA2a_test  | 2.70e-01 | 1.50e+05 | 6.03e+04 | Passed  |
BenchA2j_test  | 5.73e+08 | 1.51e+05 | 1.17e+05 | Passed  |
bench_j2a_test | 4.28e+04 | 1.44e+05 | 2.95e+05 | Passed  |
bench_j2j_test | 4.57e-01 | 0.00e+00 | 4.49e+04 | Passed  |
```
