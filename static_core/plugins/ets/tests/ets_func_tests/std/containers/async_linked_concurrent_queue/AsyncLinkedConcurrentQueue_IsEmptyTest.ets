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

function isEmptyTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    arktest.assertEQ(queue.isEmpty(), true);
}

async function isEmptyTest002(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    queue.add(new QueueValue(1));
    arktest.assertEQ(queue.isEmpty(), false);
    await queue.pop();
    arktest.assertEQ(queue.isEmpty(), true);
}

async function isEmptyStressTest003(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
    }
    arktest.assertEQ(queue.isEmpty(), false);
    for (let i = 0; i < stressNum; i++) {
        await queue.pop();
    }
    arktest.assertEQ(queue.isEmpty(), true);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueIsEmptyTest');

    testSuite.addTest('IsEmptyTest001', isEmptyTest001);
    testSuite.addAsyncTest('IsEmptyTest002', isEmptyTest002);
    testSuite.addAsyncTest('IsEmptyStressTest003', isEmptyStressTest003);

    return testSuite.run();
}