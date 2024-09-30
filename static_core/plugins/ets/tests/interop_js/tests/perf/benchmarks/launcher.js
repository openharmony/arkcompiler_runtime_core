/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let initBenchmarks = require('./caller.js').initBenchmarks;
let JSCallee = require('./callee.js');

function initArkVM() {
    let penv = process.env;
    let arkVM = require(penv.MODULE_PATH + '/ets_interop_js_napi');
    // NB: Consider setting compiler-enable-jit=false for local run
    const arkVMRes = arkVM.createRuntime({
        'log-level': 'info',
        'log-components': 'ets_interop_js',
        'boot-panda-files': penv.ARK_ETS_STDLIB_PATH + ':' + penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
        'panda-files': penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'true',
    });

    if (!arkVMRes) {
        console.error('Failed to create ETS runtime');
        abort();
    }
    return arkVM;
}


function runTest() {
    console.log('Running test ' + bench + ' (' + callerSite + '->' + calleeSite + ')');

    let benchmarks;
    if (callerSite === 'JS') {
        benchmarks = initBenchmarks(arkVM);
    } else if (callerSite === 'STS') {
        let initializer = arkVM.getFunction('LETSGLOBAL;', 'initBenchmarks');
        benchmarks = initializer(callerSite !== calleeSite);
    }
    
    let caller = benchmarks[bench];
    if (!caller) {
        throw 'Benchmark not found: \'' + bench + '\' (' + callerSite + '->' + calleeSite + ')';
    }
    const MS2NS = 1000000;

    // Warmup:
    let start = Date.now();
    caller(warmup);
    let timeNs = (Date.now() - start) * MS2NS;
    console.log('warmup ' + bench + ': ' + timeNs / warmup + ' ns/iter, iters: ' + warmup);

    start = Date.now();    
    caller(iters);
    timeNs = (Date.now() - start) * MS2NS;
    console.log('iters  ' + bench + ': ' + timeNs / iters + ' ns/iter, iters: ' + iters);

    return null;
}


globalThis.require = require;
globalThis.jsCallee = JSCallee;

let args = process.argv;
if (args.length !== 7) {
    console.log('Expected <test name> <caller-site(JS/STS)> <callee-site(JS/STS)> <warmup iters> <bench iters>');
    process.exit(1);
}
let bench = args[2];
let callerSite = args[3];
let calleeSite = args[4];

if (callerSite !== 'STS' && callerSite !== 'JS') {
    throw 'Caller site should be \'STS\' or \'JS\', got \'' + callerSite + '\'.';
}
if (calleeSite !== 'STS' && calleeSite !== 'JS') {
    throw 'Callee site should be \'STS\' or \'JS\', got \'' + calleeSite + '\'.';
}
let arkVM = (callerSite === 'STS' || calleeSite === 'STS') ? initArkVM() : undefined;
let warmup = parseInt(args[5], 10);
let iters = parseInt(args[6], 10);

runTest();
