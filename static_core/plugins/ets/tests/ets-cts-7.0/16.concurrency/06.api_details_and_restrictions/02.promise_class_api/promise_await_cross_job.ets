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
  A Promise class instance is safe to be awaited within a job on some
  worker thread while being resolved or rejected from another job on
  the same or different worker thread.
tags: [no-warmup]
---*/

import { launch, WorkerDomain, WorkerGroup, LaunchParams } from "std/concurrency"

function computeOnWorker(): int {
    return 99;
}

function failOnWorker(): int {
    throw new Error("cross-worker rejection");
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Promise await cross job");

    const params: LaunchParams = {
        workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.GENERAL)
    };

    suite.addAsyncTest("Resolve from a different worker thread", async (): Promise<void> => {
        let job: Promise<int> = launch<int>(params, computeOnWorker);
        let result: int = await job;
        arktest.assertEQ(result, 99);
    });

    suite.addAsyncTest("Reject from a different worker thread", async (): Promise<void> => {
        let job: Promise<int> = launch<int>(params, failOnWorker);
        let errorMessage: string = "";
        let result: int = await job.catch((e: Error): int => {
            errorMessage = e.message;
            return -1;
        });
        arktest.assertEQ(result, -1);
        arktest.assertEQ(errorMessage, "cross-worker rejection");
    });

    return suite.run();
}
