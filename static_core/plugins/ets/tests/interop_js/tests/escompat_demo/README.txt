Sources:
main.js - entrypoint
foo.ets / foo.ts - implementation / declaration of ets module
demo.ts - test app

Build arkruntime:
cmake -GNinja -DCMAKE_BUILD_TYPE=FastVerify -DPANDA_ETS_INTEROP_JS=1 -DCMAKE_TOOLCHAIN_FILE=../arkcompiler_runtime_core/cmake/toolchain/host_clang_14.cmake ../arkcompiler_runtime_core

Compile:
tsc --types node --typeRoots /usr/local/lib/node_modules/@types/ demo.ts foo.ts --outDir out

Launch:
ninja ets_interop_js__test_escompat_demo

Sample output:

FooFunction: check instanceof Array: true
FooFunction: zero
FooFunction: {Foo named "one"}
test: FooFunction: {Foo named "zero"},{Foo named "one"},{Foo named "two"}
test: check instanceof Array: true
test: three
test: four
test: five

Demo:

* out/main.js - launcher that initialises ArkTS VM and imports `demo`
* demo.ts - test that uses functions and classes from `foo.ets`. It does following:
    - Creates dynamic `Array` and passes it to static function `FooFunction`
    - Calls static function `BarFunction` that returns static `Array`
* foo.ets - has definitions of `FooFunction` and `BarFunction`
* foo.ts - file with TS declarations that can be generated from `foo.ets`
