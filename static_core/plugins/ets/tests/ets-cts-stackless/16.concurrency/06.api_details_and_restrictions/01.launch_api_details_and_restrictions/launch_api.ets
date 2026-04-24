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

/*---
desc: >-
  The launch API allows for defining a set of worker threads that a newly
  created job can run on via worker thread domains and groups. Worker thread
  groups are immutable sets of worker threads that can be reused across
  multiple launch calls.
tags: [no-warmup]
---*/

import { CoroutineExtras } from "std/debug/concurrency"
import { launch, WorkerDomain, WorkerGroup, LaunchParams } from "std/concurrency"

const MAIN_WORKER_ID: int = 0;

function getWorkerId(): int {
    return CoroutineExtras.getWorkerId();
}

async function main(): Promise<int> {
    let suite = new arktest.ArkTestsuite("launch API details and restrictions");

    suite.addAsyncTest("MAIN domain runs on main worker", async () => {
        const mainWorkerId: int = getWorkerId();
        arktest.assertEQ(mainWorkerId, MAIN_WORKER_ID);

        const params: LaunchParams = {
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.MAIN)
        };
        let promise: Promise<int> = launch<int>(params, getWorkerId);
        arktest.assertEQ(await promise, MAIN_WORKER_ID);
    });

    suite.addAsyncTest("GENERAL domain runs on non-main worker", async () => {
        const mainWorkerId: int = getWorkerId();
        const params: LaunchParams = {
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.GENERAL)
        };
        let promise: Promise<int> = launch<int>(params, getWorkerId);
        arktest.assertNE(await promise, mainWorkerId);
    });

    suite.addAsyncTest("Immutable group reused across multiple launches", async () => {
        const mainWorkerId: int = getWorkerId();
        const params: LaunchParams = {
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.GENERAL)
        };

        let workerIds = new Set<int>();
        for (let i = 0; i < 15; i++) {
            let promise: Promise<int> = launch<int>(params, getWorkerId);
            const workerId: int = await promise;
            arktest.assertNE(workerId, mainWorkerId);
            workerIds.add(workerId);
        }
        arktest.assertTrue(workerIds.size >= 1);
    });

    suite.addAsyncTest("Hint targets specific general worker", async () => {
        const mainWorkerId: int = getWorkerId();

        const defaultParams: LaunchParams = {
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.GENERAL)
        };
        let promise0: Promise<int> = launch<int>(defaultParams, getWorkerId);
        const targetWorkerId: int = await promise0;
        arktest.assertNE(targetWorkerId, mainWorkerId);

        const hintParams: LaunchParams = {
            workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.GENERAL, [targetWorkerId])
        };
        for (let i = 0; i < 5; i++) {
            let promise: Promise<int> = launch<int>(hintParams, getWorkerId);
            arktest.assertEQ(await promise, targetWorkerId);
        }
    });

    let res = suite.runAsync();
    return await res;
}
