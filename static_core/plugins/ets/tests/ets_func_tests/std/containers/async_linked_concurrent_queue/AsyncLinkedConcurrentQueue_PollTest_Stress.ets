/**
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

const stressNum: int = 100;

class QueueValue {
    constructor(value: int) {
        this.value = value;
    }

    public value: int;
}

let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
    new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);

async function threadsPollStressTest001(): Promise<void> {
    while (!queue.isEmpty()) {
        await queue.pop();
    }
    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
    }
    let jobs = Array.create<Job<void> | undefined>(stressNum, undefined);
    for (let i = 0; i < stressNum; i++) {
        jobs[i] = launch() {
            queue.poll();
        };
    }
    for (let i = 0; i < stressNum; i++) {
        jobs[i]!.Await();
    }
    arktest.assertEQ(queue.size, 0);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueuePollStressTest');

    testSuite.addAsyncTest('ThreadsPollStressTest001', threadsPollStressTest001);

    return testSuite.run();
}