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

{% for c in cases %}
/*---
desc: >-
  Verify that hint parameter is ignored for MAIN domain.
  Hint: {{c.desc}}
tags: [no-warmup]
---*/

import { CoroutineExtras } from "std/debug/concurrency"
import { launch, WorkerDomain, WorkerGroup, LaunchParams } from "std/concurrency"
import { Job } from "std/core"

const MAIN_WORKER_ID: int = 0;

function getWorkerId(): int {
    return CoroutineExtras.getWorkerId();
}

function main(): void {
    const mainWorkerId: int = getWorkerId();
    arktest.assertEQ(mainWorkerId, MAIN_WORKER_ID, 
        "Test precondition: main() should run on main worker (ID=0)");

    // Create hint array
    const hint: Array<int> = {{c.hint}};

    // Generate group ID with hint - hint should be ignored for MAIN domain
    const params: LaunchParams = {
        workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.MAIN, hint)
    };

    // Launch coroutine and verify it runs on main worker
    let job: Job<int> = launch<int, () => int>(getWorkerId, params);
    const launchedWorkerId: int = job.Await();

    arktest.assertEQ(launchedWorkerId, mainWorkerId, 
        "MAIN domain with hint {{c.desc}} should still execute on main worker");
}
{% endfor %}
