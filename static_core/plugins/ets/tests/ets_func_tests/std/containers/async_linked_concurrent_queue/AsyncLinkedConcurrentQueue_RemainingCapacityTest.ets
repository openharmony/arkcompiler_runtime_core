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

const elementsNum: int = 10;
const stressNum: int = 100;

class QueueValue {
    constructor(value: int) {
        this.value = value;
    }

    public value: int;
}

function remainingCapacityTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(elementsNum);
    for (let i = 0; i < 5; i++) {
        arktest.assertEQ(queue.add(new QueueValue(i)), true);
    }
    arktest.assertEQ(queue.remainingCapacity(), 5);
}

async function remainingCapacityTest002(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    for (let i = 0; i < 10; i++) {
        arktest.assertEQ(queue.remainingCapacity(), stressNum - i);
        queue.add(new QueueValue(i));
    }
    for (let i = 10; i < 20; i++) {
        await queue.push(new QueueValue(i));
    }
    arktest.assertEQ(queue.remainingCapacity(), stressNum - 20);
}

function remainingCapacityStressTest003(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    let remain = stressNum;
    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
        remain--;
        arktest.assertEQ(queue.remainingCapacity(), remain);
    }
    arktest.assertEQ(queue.size, stressNum);
}

async function remainingCapacityMixedOperationsTest004(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(elementsNum);
    arktest.assertEQ(queue.remainingCapacity(), elementsNum);

    arktest.assertEQ(queue.add(new QueueValue(0)), true);
    arktest.assertEQ(queue.remainingCapacity(), elementsNum - 1);
    await queue.push(new QueueValue(1));
    arktest.assertEQ(queue.remainingCapacity(), elementsNum - 2);
    arktest.assertEQ(queue.add(new QueueValue(2)), true);
    arktest.assertEQ(queue.remainingCapacity(), elementsNum - 3);

    arktest.assertEQ(queue.poll()!.value, 0);
    arktest.assertEQ(queue.remainingCapacity(), elementsNum - 2);
    arktest.assertEQ((await queue.pop()).value, 1);
    arktest.assertEQ(queue.remainingCapacity(), elementsNum - 1);
    arktest.assertEQ(queue.poll()!.value, 2);
    arktest.assertEQ(queue.remainingCapacity(), elementsNum);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueRemainingCapacityTest');

    testSuite.addTest('RemainingCapacityTest001', remainingCapacityTest001);
    testSuite.addAsyncTest('RemainingCapacityTest002', remainingCapacityTest002);
    testSuite.addTest('RemainingCapacityStressTest003', remainingCapacityStressTest003);
    testSuite.addAsyncTest('RemainingCapacityMixedOperationsTest004', remainingCapacityMixedOperationsTest004);

    return testSuite.run();
}