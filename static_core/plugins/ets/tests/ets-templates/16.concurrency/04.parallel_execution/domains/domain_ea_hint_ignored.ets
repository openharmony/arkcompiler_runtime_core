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
  Verify that non-EA worker IDs in hint are ignored for EA domain.
  Hint: {{c.desc}}
tags: [no-warmup]
---*/

import { CoroutineExtras } from "std/debug/concurrency"
import { launch, WorkerDomain, WorkerGroup, LaunchParams } from "std/concurrency"
import { Job, EAWorker } from "std/core"

const MAIN_WORKER_ID: int = 0;

function getWorkerId(): int {
    return CoroutineExtras.getWorkerId();
}

function main(): void {
    const mainWorkerId: int = getWorkerId();

    // Create and start an EA worker
    const ea = new EAWorker();
    ea.start();
    const eaWorkerId: int = ea.run<int>(getWorkerId).Await();

    // Verify EA worker ID is different from main
    arktest.assertNE(eaWorkerId, mainWorkerId, 
        "EA worker should have different ID than main worker");

    // Build hint array with non-EA IDs that should be ignored
    // eaWorkerId is added to ensure coroutine has valid target
    const hint: Array<int> = {{c.hint}};
    hint.push(eaWorkerId);

    // Generate group ID - non-EA IDs should be ignored
    const params: LaunchParams = {
        workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.EA, hint)
    };

    let job: Job<int> = launch<int, () => int>(getWorkerId, params);
    const launchedWorkerId: int = job.Await();

    // Verify coroutine executed on EA worker
    arktest.assertEQ(launchedWorkerId, eaWorkerId, 
        "Hint {{c.desc}}: non-EA IDs should be ignored, should execute on EA worker");

    // Cleanup
    ea.quit();
}
{% endfor %}
