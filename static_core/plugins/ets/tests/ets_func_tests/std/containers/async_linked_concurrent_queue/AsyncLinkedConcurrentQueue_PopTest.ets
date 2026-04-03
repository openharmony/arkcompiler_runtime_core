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

async function popAndStore(queue: containers.stackless.LinkedConcurrentQueue<QueueValue>, results: int[], id: int,
    completed: AtomicInt): Promise<void> {
    results[id] = (await queue.pop()).value;
    completed.fetchAdd(1);
}

async function popAndMark(queue: containers.stackless.LinkedConcurrentQueue<QueueValue>, poppedValue: AtomicInt,
    popCompleted: AtomicBoolean): Promise<void> {
    poppedValue.store((await queue.pop()).value);
    popCompleted.store(true);
}

async function popTest001(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < elementsNum; i++) {
        queue.add(new QueueValue(i));
    }
    for (let i = 0; i < elementsNum; i++) {
        arktest.assertEQ((await queue.pop()).value, i);
    }
    arktest.assertEQ(queue.isEmpty(), true);
    arktest.assertEQ(queue.size, 0);
}

async function popStressTest002(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
    }
    for (let i = 0; i < stressNum; i++) {
        await queue.pop();
    }
    arktest.assertEQ(queue.size, 0);
}

async function popBlocksUntilPushTest003(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    let popCompleted = new AtomicBoolean(false);
    let poppedValue = new AtomicInt(-1);

    let consumer = popAndMark(queue, poppedValue, popCompleted);

    for (let i = 0; i < elementsNum; i++) {
        Coroutine.Schedule();
    }
    arktest.assertFalse(popCompleted.load());

    await queue.push(new QueueValue(elementsNum));
    await consumer;
    arktest.assertTrue(popCompleted.load());
    arktest.assertEQ(poppedValue.load(), elementsNum);
    arktest.assertTrue(queue.isEmpty());
}

async function popWaitersResumeOneByOneTest004(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    const waiterCount: int = 2;
    let completed = new AtomicInt(0);
    let results = Array.create<int>(waiterCount, -1);
    let jobs = Array.create<Promise<void> | undefined>(waiterCount, undefined);

    for (let i = 0; i < waiterCount; i++) {
        const id = i;
        jobs[id] = popAndStore(queue, results, id, completed);
    }

    for (let i = 0; i < elementsNum; i++) {
        Coroutine.Schedule();
    }
    arktest.assertEQ(completed.load(), 0);

    await queue.push(new QueueValue(0));
    for (let i = 0; i < stressNum && completed.load() == 0; i++) {
        Coroutine.Schedule();
    }
    arktest.assertTrue(completed.load() < waiterCount);

    await queue.push(new QueueValue(1));
    for (let i = 0; i < waiterCount; i++) {
        await jobs[i]!;
    }

    let counts = new FixedArray<int>(waiterCount);
    for (let i = 0; i < waiterCount; i++) {
        arktest.assertTrue(results[i] >= 0 && results[i] < waiterCount);
        counts[results[i]] += 1;
    }
    arktest.assertEQ(counts[0], 1);
    arktest.assertEQ(counts[1], 1);
    arktest.assertTrue(queue.isEmpty());
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueuePopTest');

    testSuite.addAsyncTest('PopTest001', popTest001);
    testSuite.addAsyncTest('PopStressTest002', popStressTest002);
    testSuite.addAsyncTest('PopBlocksUntilPushTest003', popBlocksUntilPushTest003);
    testSuite.addAsyncTest('PopWaitersResumeOneByOneTest004', popWaitersResumeOneByOneTest004);

    return testSuite.run();
}