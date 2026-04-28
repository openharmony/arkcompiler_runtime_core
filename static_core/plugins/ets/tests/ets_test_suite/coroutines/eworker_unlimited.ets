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

// Using one worker in runtime options allows to use 127 free worker id's
const NUM_EWORKERS: int = 127;

function CreateWorkers(): Array<EAWorker> {
    let workers = new Array<EAWorker>();
    for (let i = 0; i < NUM_EWORKERS; i++) {
        let eaWorker = new EAWorker();
        eaWorker.start();
        workers.push(eaWorker);
    }
    return workers;
}

function CheckLimitReached() {
    let catched = false;
    try {
        let w = new EAWorker();
        w.start();
        w.quit();
    } catch(e) {
        catched = true;
    }

    return catched;
}

function LoadWorkers(workers: Array<EAWorker>): Array<Promise<int>> {
    let promises = new Array<Promise<int>>();

    for (let i = 0; i < NUM_EWORKERS; i++) {
        let promise = workers[i].run<int>(():int => {
            let runId = CoroutineExtras.getWorkerId();
            arktest.assertNE(runId, mainId);
            return runId;
        });
        promises.push(promise);
    }

    return promises;
}

async function AwaitPromises(promises: Array<Promise<int>>): Promise<Array<int>> {
    let ids = Array<int>();

    for (let i = 0; i < NUM_EWORKERS; i++) {
        let id = await promises[i];
        ids.push(id);
    }

    return ids;
}

function JoinWorkers(workers: Array<EAWorker>) {
    workers.forEach((eaWorker: EAWorker) => {
        eaWorker.quit();
    });
}

function CheckUnique(arr: Array<int>) {
    let s = new Set<int>(arr);
    arktest.assertEQ(s.size, arr.length);
}

async function testEworkerUnlimited(): Promise<void> {
    let workers = CreateWorkers();

    if (!CheckLimitReached()) {
        JoinWorkers(workers);
        arktest.assertTrue(false, 'EAWorkers limit should be reached before');
    }

    let promises = LoadWorkers(workers);
    let ids = await AwaitPromises(promises);
    CheckUnique(ids);
    JoinWorkers(workers);
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.eworker_unlimited');
    testSuite.addAsyncTest('testEworkerUnlimited', testEworkerUnlimited);
    let res = testSuite.run();
    return res;
}
