How to run
==========

# assume PANDA and VMB are built and installed per $PANDA_ROOT/tests/vm-benchmarks/interop.readme.md
cd $PANDA_ROOT/tests/vm-benchmarks

# VMB needs variables PANDA_SRC and PANDA_BUILD for these benchmarks
export PANDA_SRC=$PANDA_ROOT
export PANDA_BUILD=$PANDA_ROOT/build

vmb all -p arkts_host --aot-skip-libs -l sts -L sts -v debug $PANDA_SRC/plugins/ets/tests/benchmarks/ani

Example of the output of successful run
=======================================

arkts_host-default
==================

7 tests; 0 failed; 0 excluded; Time(GM): 3913.0 Size(GM): 161525.1 RSS(GM): 58032.7
===================================================================================


Test                                   |   Time   | CodeSize |   RSS    | Status  |
===================================================================================
BasicCallStatic_criticalCallBench      | 3.80e+02 | 1.62e+05 | 5.48e+04 | Passed  |
BasicCallStatic_fastStaticCallBench    | 5.59e+03 | 1.62e+05 | 5.86e+04 | Passed  |
BasicCallStatic_finalCallBench         | 5.83e+03 | 1.62e+05 | 5.87e+04 | Passed  |
BasicCallStatic_finalFastCallBench     | 6.48e+03 | 1.62e+05 | 5.84e+04 | Passed  |
BasicCallStatic_nonStaticCallBench     | 5.53e+03 | 1.62e+05 | 5.87e+04 | Passed  |
BasicCallStatic_nonStaticFastCallBench | 6.11e+03 | 1.62e+05 | 5.86e+04 | Passed  |
BasicCallStatic_staticCallBench        | 5.18e+03 | 1.62e+05 | 5.86e+04 | Passed  |
