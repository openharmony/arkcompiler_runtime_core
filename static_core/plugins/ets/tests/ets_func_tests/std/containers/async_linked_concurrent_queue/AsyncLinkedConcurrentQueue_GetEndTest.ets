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

function getEndTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < elementsNum; i++) {
        queue.add(new QueueValue(i));
    }
    arktest.assertEQ(queue.getEnd()!.value, elementsNum - 1);
    arktest.assertEQ(queue.size, elementsNum);
}

async function getEndTest002(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < elementsNum; i++) {
        queue.add(new QueueValue(i));
    }
    await queue.pop();
    arktest.assertEQ(queue.getEnd()!.value, elementsNum - 1);
    arktest.assertEQ(queue.size, elementsNum - 1);
}

async function getEndConcurrentTest003(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    let observedDecrease = new AtomicBoolean(false);
    let lastSeen = new AtomicInt(-1);

    let observer = launch() {
        for (let i = 0; i < stressNum * 2; i++) {
            let end = queue.getEnd();
            if (end != undefined) {
                let value = end!.value;
                if (value < lastSeen.load()) {
                    observedDecrease.store(true);
                }
                lastSeen.store(value);
            }
        }
    };

    for (let i = 0; i < stressNum; i++) {
        await queue.push(new QueueValue(i));
    }

    observer.Await();
    arktest.assertFalse(observedDecrease.load());
    arktest.assertEQ(queue.getEnd()!.value, stressNum - 1);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueGetEndTest');

    testSuite.addTest('GetEndTest001', getEndTest001);
    testSuite.addAsyncTest('GetEndTest002', getEndTest002);
    testSuite.addAsyncTest('GetEndConcurrentTest003', getEndConcurrentTest003);

    return testSuite.run();
}