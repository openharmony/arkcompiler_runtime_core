How to run
----------

cd $PANDA_ROOT/tests/vm-benchmarks
# assume PANDA and VMB are built and installed per $PANDA_ROOT/tests/vm-benchmarks/interop.readme.md
export PANDA_BUILD=$PANDA_ROOT/build
export PANDA_STDLIB_SRC=$PANDA_ROOT/plugins/ets/stdlib
vmb all -p arkts_node_interop_host --aot-skip-libs -v debug \
  --ark-custom-option=--gc-trigger-type=heap-trigger \
  --ark-custom-option=--compiler-enable-jit=true \
  --ark-custom-option=--run-gc-in-place=false \
  --ark-custom-option=--log-components=ets_interop_js \
  --ark-custom-option=--load-runtimes=ets \
  --exclude-list ../../plugins/ets/tests/benchmarks/interop_js/exclude-interop-benchmarks.txt ../../plugins/ets/tests/benchmarks/interop_js

How to run for ArkjsVM
----------------------

cd $PANDA_ROOT/tests/vm-benchmarks
# assume ArkjsVM built and installed per $PANDA_ROOT/plugins/ets/tests/interop_js/README.MD
# assume PANDA and VMB are built and installed per $PANDA_ROOT/tests/vm-benchmarks/interop.readme.md
export PANDA_BUILD=$PANDA_ROOT/build
export PANDA_STDLIB_SRC=$PANDA_ROOT/plugins/ets/stdlib
vmb all -p arkts_arkjs_interop_host --aot-skip-libs -v debug \
  --ark-custom-option=--gc-trigger-type=heap-trigger \
  --ark-custom-option=--compiler-enable-jit=true \
  --ark-custom-option=--run-gc-in-place=false \
  --ark-custom-option=--log-components=ets_interop_js \
  --ark-custom-option=--load-runtimes=ets \
  --exclude-list ../../plugins/ets/tests/benchmarks/interop_js/exclude-interop-benchmarks.txt ../../plugins/ets/tests/benchmarks/interop_js

If needed, refer VMB documentation for more details of how to use, configure, and develop these benchmarks.

Also, additional details about interop benchmarks can be found in $PANDA_ROOT/tests/vm-benchmarks/examples/benchmarks-interop-templates/README.md
