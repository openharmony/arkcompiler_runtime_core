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

function pollTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < elementsNum; i++) {
        queue.add(new QueueValue(i));
    }
    for (let i = 0; i < elementsNum; i++) {
        arktest.assertEQ(queue.poll()!.value, i);
    }
    arktest.assertEQ(queue.poll(), undefined);
}

function pollStressTest002(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
    }
    for (let i = 0; i < stressNum; i++) {
        arktest.assertEQ(queue.poll()!.value, i);
    }
    arktest.assertEQ(queue.size, 0);
}

function pollEmptyTest003(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    arktest.assertEQ(queue.poll(), undefined);
    arktest.assertEQ(queue.poll(), undefined);
    arktest.assertEQ(queue.poll(), undefined);
}

async function concurrentPopPollTest004(): Promise<void> {
    const queueSize: int = 20;
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(queueSize);
    let popped = Array.create<Promise<QueueValue>>(0, Promise.resolve(new QueueValue(0)));
    let counts = new FixedArray<int>(queueSize);

    for (let i = 0; i < queueSize; i++) {
        queue.add(new QueueValue(i));
    }
    for (let i = 0; i < queueSize / 2; i++) {
        popped.push(queue.pop());
    }
    for (let i = 0; i < queueSize / 2; i++) {
        let element = queue.poll();
        arktest.assertTrue(element != undefined);
        counts[element!.value] += 1;
    }

    let results = await Promise.all(popped);
    for (let i = 0; i < results.length; i++) {
        counts[results[i].value] += 1;
    }
    for (let i = 0; i < queueSize; i++) {
        arktest.assertEQ(counts[i], 1);
    }
    arktest.assertEQ(queue.size, 0);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueuePollTest');

    testSuite.addTest('PollTest001', pollTest001);
    testSuite.addTest('PollStressTest002', pollStressTest002);
    testSuite.addTest('PollEmptyTest003', pollEmptyTest003);
    testSuite.addAsyncTest('ConcurrentPopPollTest004', concurrentPopPollTest004);

    return testSuite.run();
}