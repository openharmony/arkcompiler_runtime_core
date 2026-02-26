/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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


let event_coro = new CompletableJob<int>()
let event_main = new CompletableJob<int>()
const cnt = 100

async function test() {
    event_coro.Await()
    return 1
}

async function testCoro() {
    const start_count = CoroutineExtras.getCoroutineCount()

   let arr: Array<Promise<int>> = new Array<Promise<int>>()

    for (let i = 0; i < cnt; ++i) {
        arr.push(test())
    }
    let count = CoroutineExtras.getCoroutineCount()

    arktest.assertEQ(start_count + cnt, count)

    event_coro.finish(0)

    for (let i = 0; i < cnt; ++i) {
        await arr[i]
    }

    count = CoroutineExtras.getCoroutineCount()

    arktest.assertEQ(start_count, count)

    event_main.finish(0)
 
}

async function  payload ()  {
        const start_count = CoroutineExtras.getCoroutineCount()
        testCoro()
        event_main.Await()
        arktest.assertEQ(start_count , CoroutineExtras.getCoroutineCount())
}   

function main() {
    let testsuite = new arktest.ArkTestsuite("CoroCount");
    testsuite.addAsyncTest('testCoroCount', payload)
    return testsuite.run();
}
