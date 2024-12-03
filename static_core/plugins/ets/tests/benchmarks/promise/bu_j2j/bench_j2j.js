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

async function main() {
    console.log('Started...');
    const iterations = 1000000; // this value worked reasonably well in trial runs

    const timeNs = await checkPromise();
    console.log('Benchmark result: J2j ' + (timeNs));
}

async function checkPromise() {
    let start;
    let loopTime = 0;

    for (let i = 0; i < iterations; i++) {
        await (() => {
            return new Promise((resolve) => {
                start = process.hrtime.bigint();
                resolve();
            });
        }
        )().then(() => {
            loopTime += Number(process.hrtime.bigint() - start);
        });
    }

    return loopTime;
}

main();
