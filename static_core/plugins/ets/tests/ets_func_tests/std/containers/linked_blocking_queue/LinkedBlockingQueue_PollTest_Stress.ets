/**
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

import { launch } from "std/concurrency"
import { Job } from "std/core"

const stressNum: int = 100;

let sMTQueueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);

function main(): int {
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueuePollTest");

    LinkedBlockingQueueTestsuite.addTest("ThreadsPollStressTest003", () =>
    {
        while (!sMTQueueInt.isEmpty()) {
            sMTQueueInt.pop();
        }
        for (let i = 0; i < stressNum; i++) {
            sMTQueueInt.add(i);
        }
        let jobs = new Array<Job<void>>(100);
        let p = 0;
        for (let i = 0; i < stressNum; i++) {
            jobs[p++] = launch<void, () => void>(sQueuePollData);
        }
        for (let i: int = 0; i < 100; i++) {
            jobs[i].Await();
        }
        assertEQ(sMTQueueInt.size, 0);
    });

    return LinkedBlockingQueueTestsuite.run();
}

function sQueuePollData() {
    sMTQueueInt.poll();
}