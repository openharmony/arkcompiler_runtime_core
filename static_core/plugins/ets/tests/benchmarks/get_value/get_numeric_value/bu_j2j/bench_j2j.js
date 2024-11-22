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

const { 
    jsNumber,
    jsByte,
    jsShort,
    jsInt,
    jsLong,
    jsFloat,
    jsDouble
} = require('./test_import');

function main() {
    console.log('Starting...');

    let totalTime = 0n;
    for (let i = 0; i < 10000; i++) { // Number of iterations was picked experimentally
        let start = process.hrtime.bigint();
        let numVal = jsNumber;
        let byteVal = jsByte;
        let shortVal = jsShort;
        let intVal = jsInt;
        let longVal = jsLong;
        let floatVal = jsFloat;
        let doubleVal = jsDouble;
        totalTime += (process.hrtime.bigint() - start);
        delete require.cache['./test_import'];
    }
    console.log('Benchmark result: J2j ' + totalTime);
}

main();
