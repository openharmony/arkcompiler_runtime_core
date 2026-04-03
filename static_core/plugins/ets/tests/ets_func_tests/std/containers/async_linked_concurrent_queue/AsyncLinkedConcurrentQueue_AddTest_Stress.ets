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
const queueCapacity: int = 16;

class QueueValue {
    constructor(value: int) {
        this.value = value;
    }

    public value: int;
}

let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
    new containers.stackless.LinkedConcurrentQueue<QueueValue>(queueCapacity);

async function threadsAddStressTest001(): Promise<void> {
    while (!queue.isEmpty()) {
        await queue.pop();
    }
    let successCount = new AtomicInt(0);
    let failureCount = new AtomicInt(0);
    let jobs = Array.create<Job<void> | undefined>(stressNum, undefined);
    for (let i = 0; i < stressNum; i++) {
        jobs[i] = launch() {
            if (queue.add(new QueueValue(i))) {
                successCount.fetchAdd(1);
            } else {
                failureCount.fetchAdd(1);
            }
        };
    }
    for (let i = 0; i < stressNum; i++) {
        jobs[i]!.Await();
    }
    arktest.assertEQ(successCount.load(), queueCapacity);
    arktest.assertEQ(failureCount.load(), stressNum - queueCapacity);
    arktest.assertEQ(queue.size, queueCapacity);

    let seen = new FixedArray<int>(stressNum);
    while (!queue.isEmpty()) {
        let element = queue.poll();
        arktest.assertTrue(element != undefined);
        let index = element!.value;
        arktest.assertEQ(seen[index], 0);
        seen[index] = 1;
    }
    arktest.assertTrue(queue.isEmpty());
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueAddStressTest');

    testSuite.addAsyncTest('ThreadsAddStressTest001', threadsAddStressTest001);

    return testSuite.run();
}