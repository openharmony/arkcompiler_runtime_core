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

const { jsVoid, returnAnonymous } = require('./import');

function main() {
    console.log('Starting...');

    const iterations = 1000000;
    let totalTime = 0;

    function callFunction(fun, target) {
        let start;
        let loopTime = 0;

        for (let i = 0; i < iterations; i++) {
            start = process.hrtime.bigint();
            fun(iterations, iterations);
            loopTime += Number(process.hrtime.bigint() - start);
        }

        totalTime += loopTime;
        console.log(`Benchmark result: J2j ${target} ` + loopTime);
    }

    const anonymous = returnAnonymous();

    callFunction(anonymous, 'anonymous fu');

    callFunction(jsVoid, 'function');

    console.log('Benchmark result: J2j ' + totalTime);

    return null;
}

main();
