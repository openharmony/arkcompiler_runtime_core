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

function capacityTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(elementsNum);
    arktest.assertEQ(queue.capacity, elementsNum);
}

function capacityTest002(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    arktest.assertEQ(queue.capacity, Int.MAX_VALUE);
}

async function capacityTest003(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(elementsNum);
    for (let i = 0; i < elementsNum / 2; i++) {
        await queue.push(new QueueValue(i));
    }
    arktest.assertEQ(queue.capacity, elementsNum);
}

function getCapacityStressTest004(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    arktest.assertEQ(queue.capacity, stressNum);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueCapacityTest');

    testSuite.addTest('CapacityTest001', capacityTest001);
    testSuite.addTest('CapacityTest002', capacityTest002);
    testSuite.addAsyncTest('CapacityTest003', capacityTest003);
    testSuite.addTest('GetCapacityStressTest004', getCapacityStressTest004);

    return testSuite.run();
}