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

//NOTE: if there is no sts file in the directory then vmb cannot open the js file

const {
    jsBigInt,
    jsBool,
    jsNull,
    jsUndefined,
} = require('./import');

function main() {
    const noEqual = '===';
    const equal = '!==';
    const MS2NS = 1000000;

    function message(data) {
        console.log('Benchmark result: equality_primitives_j2j ' + data);
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

    // NOTE issue (17741) - enable below after fix import bigInt
    if (false) {
        comparison(jsBigInt, jsBigInt, equal, 'BigInt ');
    }

    comparison(jsBool, jsBool, equal, 'boolean ');

    comparison(jsNull, jsNull, equal, 'null ');

    // NOTE: (19193) enable below after fix comparison
    if (false) {
        comparison(jsUndefined, jsUndefined, equal, 'undefined ');
    }

    message(totalTime);
}

main();
