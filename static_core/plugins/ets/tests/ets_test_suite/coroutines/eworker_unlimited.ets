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

let mainId = CoroutineExtras.getWorkerId();

// Using one worker in runtime options allows to use 63 free worker id's
const NUM_EWORKERS: int = 63;

function CreateWorkers(): Array<EAWorker> {
    let workers = new Array<EAWorker>();
    for (let i = 0; i < NUM_EWORKERS; i++) {
        workers.push(new EAWorker());
    }
    return workers;
}

function CheckLimitReached() {
    let catched = false;
    try {
        let w = new EAWorker();
        w.join();
    } catch(e) {
        catched = true;
    }

    return catched;
}

function LoadWorkers(workers: Array<EAWorker>): Array<Job<int>> {
    let jobs = new Array<Job<int>>();

    for (let i = 0; i < NUM_EWORKERS; i++) {
        let job = workers[i].run<int>(():int => {
            let runId = CoroutineExtras.getWorkerId();
            assertNE(runId, mainId);
            return runId;
        });
        jobs.push(job);
    }

    return jobs;
}

function AwaitJobs(jobs: Array<Job<int>>): Array<int> {
    let ids = Array<int>();

    for (let i = 0; i < NUM_EWORKERS; i++) {
        let id = jobs[i].Await();
        ids.push(id);
    }

    return ids;
}

function JoinWorkers(workers: Array<EAWorker>) {
    workers.forEach((eaWorker: EAWorker) => {
        eaWorker.join();
    });
}

function CheckUnique(arr: Array<int>) {
    let s = new Set<int>(arr);
    assertEQ(s.size, arr.length);
}

function main() {
    let workers = CreateWorkers();

    if (!CheckLimitReached()) {
        JoinWorkers(workers);
        assertTrue(false, 'EAWorkers limit should be reached before');
    }

    let jobs = LoadWorkers(workers);
    let ids = AwaitJobs(jobs);
    CheckUnique(ids);
    JoinWorkers(workers);
}
