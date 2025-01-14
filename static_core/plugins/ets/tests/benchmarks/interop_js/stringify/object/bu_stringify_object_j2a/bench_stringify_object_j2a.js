/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

    const iterations = 1000000;
    let totalTime = 0;

    const Person = stsVm.getClass('LStringifyObjectJ2a;');

    const obj = new Person();

    let start;
    let loopTime = 0;

    for (let i = 0; i < iterations; i++) {
        start = process.hrtime.bigint();
        JSON.stringify(obj);
        loopTime += Number(process.hrtime.bigint() - start);
    }

    totalTime += loopTime;

    console.log('Benchmark result: stringify_object_j2a ' + totalTime);

    return null;
}

main();
