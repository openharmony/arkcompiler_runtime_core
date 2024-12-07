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
        console.log('Benchmark result: J2a ' + data);
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

    const State = stsVm.getClass('LA2j;');

    const foo = stsVm.getFunction('LETSGLOBAL;', 'foo');
    const fooOther = stsVm.getFunction('LETSGLOBAL;', 'fooOther');

    const bench = new State();

    // NOTE issue (17741) - enable below after fix import bigInt
    if (false) {
        comparison(bench.stsBigInt, bench.stsBigInt, equal, 'BigInt ');
    }

    comparison(bench.stsBool, bench.stsBool, equal, 'boolean ');

    comparison(bench.null, bench.null, equal, 'null ');

    // NOTE: (19193) enable below after fix comparison
    if (false) {
        comparison(bench.stsUndefined, bench.stsUndefined, equal, 'undefined ');
    }

    message(totalTime);

    return null;
}

main();
