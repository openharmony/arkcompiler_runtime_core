/*
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

function main() {
    const seed = 123;
    let strNumber = '';
    console.log('Starting...');
    let penv = process.env;
    let stsVm = require(penv.MODULE_PATH + '/ets_interop_js_napi.node');
    const stsRT = stsVm.createRuntime({
        'boot-panda-files': penv.ARK_ETS_STDLIB_PATH + ':' + penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
        'panda-files': penv.ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
        'gc-trigger-type': 'heap-trigger',
        'compiler-enable-jit': 'false',
        'run-gc-in-place': 'false',
    });

    if (!stsRT) {
        console.error('Failed to create ETS runtime');
        return 1;
    }

    function generateNumber(seed) {
        const modulus = Math.pow(2, 32);
        const a = 1664525;
        const c = 1013904223;
      
        seed = (a * seed + c) % modulus;
        
        return Math.floor((seed / modulus) * 100); 
    }
    
    const data = generateNumber(seed);
    strNumber = data.toString(10);
    const State = stsVm.getClass('LConversionDecimalJ2a;');

    const start = process.hrtime.bigint();
    let bench = new State();
    bench.setup();

    for (let i = 0; i < 10000; i++) {
        bench.test(strNumber);
    }
    const end = process.hrtime.bigint();
    let time_ns = end - start;
    console.log('Benchmark result: conversion_decimal_j2a ' + time_ns);

    return null;
}

main();
