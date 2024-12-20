How to run

cd $PANDA_ROOT/tests/vm-benchmarks
# assume PANDA and VMB are built and installed per $PANDA_ROOT/tests/vm-benchmarks/interop.readme.md
export PANDA_BUILD=$PANDA_ROOT/build
vmb run -p arkts_node_interop_host -v debug --exclude-list ../../plugins/ets/tests/benchmarks/interop_js/exclude-interop-benchmarks.txt ../../plugins/ets/tests/benchmarks/interop_js

Note that some benchmarks may need to have corrected compilation executed prior to run due to limitations of VMB (#697), like:

env LD_LIBRARY_PATH=$PANDA_ROOT/build/lib $PANDA_ROOT/build/bin/es2panda --stdlib=$PANDA_ROOT/plugins/ets/stdlib --extension=sts --opt-level=2 --arktsconfig=/tmp/arktsconfig_w5m2tk70.json --output=$PANDA_ROOT/plugins/ets/tests/benchmarks/interop_js/call_import_function/bu_call_import_function_j2j/bench_call_import_function_j2j.abc $PANDA_ROOT/plugins/ets/tests/benchmarks/interop_js/call_import_function/bu_call_import_function_j2j/bench_call_import_function_j2j.sts
