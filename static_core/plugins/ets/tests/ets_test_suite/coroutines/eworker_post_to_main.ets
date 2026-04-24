/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

 let mainId = CoroutineExtras.getWorkerId();

async function testEworkerPost2Main(): Promise<void> {
    let p1 = EAWorker.postToMain<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertEQ(runId, mainId);
        return runId;
    });

    let p2 = EAWorker.postToMain<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertEQ(runId, mainId);
        return runId;
    });

    let p3 = launch<int>() {
        let runId = CoroutineExtras.getWorkerId();
        arktest.assertNE(runId, mainId);
        return runId;
    };

    let runId1 = await p1;
    let runId2 = await p2;
    let runId3 = await p3;

    arktest.assertEQ(runId1, runId2);
    arktest.assertNE(runId1, runId3);
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.eworker_post_to_main');
    testSuite.addAsyncTest('testEworkerPost2Main', testEworkerPost2Main);
    let res = testSuite.run();
    return res;
}
