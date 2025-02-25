How to run
----------

cd $PANDA_ROOT/tests/vm-benchmarks
# assume PANDA and VMB are built and installed per $PANDA_ROOT/tests/vm-benchmarks/interop.readme.md
export PANDA_BUILD=$PANDA_ROOT/build
export PANDA_STDLIB_SRC=$PANDA_ROOT/plugins/ets/stdlib
vmb all -p arkts_node_interop_host --aot-skip-libs -v debug --exclude-list ../../plugins/ets/tests/benchmarks/interop_js/exclude-interop-benchmarks.txt ../../plugins/ets/tests/benchmarks/interop_js

If needed, refer VMB documentation for more details of how to use, configure, and develop these benchmarks.
