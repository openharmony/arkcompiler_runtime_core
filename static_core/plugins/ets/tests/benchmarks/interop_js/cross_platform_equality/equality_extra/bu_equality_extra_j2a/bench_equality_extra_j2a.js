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

    const noEqual = '===';
    const equal = '!==';
    const MS2NS = 1000000;

    function message(data) {
        console.log('Benchmark result: equality_extra_j2a ' + data);
    }

    let totalTime = 0;
    function comparison(valueA, valueB, compare, target) {
        let start;

        let loopTime = 0;
        for (let i = 0; i < MS2NS; i++) {
            start = process.hrtime.bigint();

            if (compare === equal && valueA !== valueB) {
                throw Error();
            }
            if (compare === noEqual && valueA === valueB) {
                throw Error();
            }

            loopTime += Number(process.hrtime.bigint() - start);
        }

        totalTime += loopTime;
        message(target + loopTime);
    }

    const State = stsVm.getClass('LEqualityExtraJ2a;');

    const foo = stsVm.getFunction('LETSGLOBAL;', 'foo');
    const fooOther = stsVm.getFunction('LETSGLOBAL;', 'fooOther');

    const bench = new State();

    //NOTE: issue(19194) enable below after fix compare arrays
    if (false) {
        comparison(bench.stsArr, bench.stsArr, equal, 'array ');
        comparison(bench.stsArr, bench.stsArrCopy, equal, 'array by link ');
    }

    comparison(bench.stsArr, bench.stsArrOther, noEqual, 'array different array ');

    comparison(bench.stsArrowFunction, bench.stsArrowFunction, equal, 'arrow function ');

    comparison(bench.stsArrowFunction, bench.stsArrowFunctionCopy, equal, 'arrow function by link ');

    comparison(bench.stsArrowFunction, bench.stsArrowFunctionOther, noEqual, 'arrow different arrow ');

    //NOTE: issue (19133) enable below after fix comparison functions
    if (false) {
        comparison(foo, foo, equal, 'function with itself ');
        comparison(foo, fooOther, noEqual, 'function different function ');
    }

    message(totalTime);

    return null;
}

main();