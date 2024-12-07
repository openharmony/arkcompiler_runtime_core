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
    const MS2NS = 1000000;
    let totalTime = 0;

    const array = [1, 2, 3, 4, 5];
    class Person {
        name = 'string';
        constructor() {
        }
    }
    const obj = new Person();

    function toStringify(arg, target) {
        let start;
        let loopTime = 0;

        for (let i = 0; i < MS2NS; i++) {
            start = process.hrtime.bigint();
            JSON.stringify(arg);
            loopTime += Number(process.hrtime.bigint() - start);
        }

        console.log(`Benchmark result: J2j ${target} ` + loopTime);
        totalTime += loopTime;
    }

    toStringify(array, 'array');


    toStringify(obj, 'object');

    console.log('Benchmark result: J2j ' + totalTime);
}

main();
