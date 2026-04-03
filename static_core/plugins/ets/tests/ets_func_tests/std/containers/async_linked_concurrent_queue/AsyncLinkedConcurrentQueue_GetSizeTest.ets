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

function sizeTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    arktest.assertEQ(queue.size, 0);
}

async function sizeTest002(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < elementsNum; i++) {
        queue.add(new QueueValue(i));
    }
    arktest.assertEQ(queue.size, elementsNum);
    await queue.pop();
    arktest.assertEQ(queue.size, elementsNum - 1);
    queue.poll();
    arktest.assertEQ(queue.size, elementsNum - 2);
}

async function getSizeStressTest003(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
    }
    arktest.assertEQ(queue.size, stressNum);
    for (let i = 0; i < stressNum; i++) {
        await queue.pop();
    }
    arktest.assertEQ(queue.size, 0);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueSizeTest');

    testSuite.addTest('SizeTest001', sizeTest001);
    testSuite.addAsyncTest('SizeTest002', sizeTest002);
    testSuite.addAsyncTest('GetSizeStressTest003', getSizeStressTest003);

    return testSuite.run();
}