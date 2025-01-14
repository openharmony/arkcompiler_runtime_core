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
    const iterations = 1000000;
    let totalTime = 0;

    const array = [1, 2, 3, 4, 5];

    let start;
    let loopTime = 0;

    for (let i = 0; i < iterations; i++) {
        start = process.hrtime.bigint();
        JSON.stringify(array);
        loopTime += Number(process.hrtime.bigint() - start);
    }

    totalTime += loopTime;

    console.log('Benchmark result: stringify_array_j2j ' + totalTime);
}

main();
