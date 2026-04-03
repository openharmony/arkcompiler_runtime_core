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

function getFirstTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < elementsNum; i++) {
        queue.add(new QueueValue(i));
    }
    arktest.assertEQ(queue.getFirst()!.value, 0);
    arktest.assertEQ(queue.size, elementsNum);
}

async function getFirstTest002(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < elementsNum; i++) {
        queue.add(new QueueValue(i));
    }
    await queue.pop();
    arktest.assertEQ(queue.getFirst()!.value, 1);
    arktest.assertEQ(queue.size, elementsNum - 1);
}

async function getFirstStressTest003(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
    }
    for (let i = 0; i < stressNum; i++) {
        arktest.assertEQ(queue.getFirst()!.value, i);
        await queue.pop();
    }
    arktest.assertEQ(queue.size, 0);
}

async function getFirstConcurrentTest004(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    let observedDecrease = new AtomicBoolean(false);
    let lastSeen = new AtomicInt(-1);

    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
    }

    let observer = launch() {
        while (true) {
            let first = queue.getFirst();
            if (first == undefined) {
                break;
            }
            let value = first!.value;
            if (value < lastSeen.load()) {
                observedDecrease.store(true);
            }
            lastSeen.store(value);
            Coroutine.Schedule();
        }
    };

    while (!queue.isEmpty()) {
        let first = queue.getFirst();
        arktest.assertEQ(first!.value, (await queue.pop()).value);
        Coroutine.Schedule();
    }

    observer.Await();
    arktest.assertFalse(observedDecrease.load());
    arktest.assertEQ(queue.size, 0);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueGetFirstTest');

    testSuite.addTest('GetFirstTest001', getFirstTest001);
    testSuite.addAsyncTest('GetFirstTest002', getFirstTest002);
    testSuite.addAsyncTest('GetFirstStressTest003', getFirstStressTest003);
    testSuite.addAsyncTest('GetFirstConcurrentTest004', getFirstConcurrentTest004);

    return testSuite.run();
}