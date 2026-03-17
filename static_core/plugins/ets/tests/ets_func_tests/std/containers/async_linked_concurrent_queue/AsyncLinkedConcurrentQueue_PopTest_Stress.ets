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
const producerCount: int = 2;
const consumerCount: int = 2;
const valuesPerProducer: int = 20;
const totalValues: int = producerCount * valuesPerProducer;

class QueueValue {
    constructor(value: int) {
        this.value = value;
    }

    public value: int;
}

async function threadsPopStressTest001(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    for (let i = 0; i < stressNum; i++) {
        queue.add(new QueueValue(i));
    }
    let promises = Array.create<Promise<QueueValue>>(0, Promise.resolve(new QueueValue(0)));
    for (let i = 0; i < stressNum; i++) {
        promises.push(queue.pop());
    }
    let results = await Promise.all(promises);
    arktest.assertEQ(results.length, stressNum);
    arktest.assertEQ(queue.size, 0);
}

async function producer(queue: containers.stackless.LinkedConcurrentQueue<QueueValue>, start: int,
    count: int, produced: AtomicInt): Promise<void> {
    for (let i = 0; i < count; i++) {
        await queue.push(new QueueValue(start + i));
        produced.fetchAdd(1);
        Coroutine.Schedule();
    }
}

async function consumer(queue: containers.stackless.LinkedConcurrentQueue<QueueValue>, count: int): Promise<FixedArray<int>> {
    let seen = new FixedArray<int>(totalValues);
    for (let i = 0; i < count; i++) {
        let value = (await queue.pop()).value;
        seen[value] += 1;
        Coroutine.Schedule();
    }
    return seen;
}

async function observeQueue(queue: containers.stackless.LinkedConcurrentQueue<QueueValue>, produced: AtomicInt,
    observerError: AtomicBoolean): Promise<void> {
    while (produced.load() < totalValues || !queue.isEmpty()) {
        let first = queue.getFirst();
        let end = queue.getEnd();
        if (first != undefined) {
            let firstValue = first!.value;
            if (firstValue < 0 || firstValue >= totalValues) {
                observerError.store(true);
            }
        }
        if (end != undefined) {
            let endValue = end!.value;
            if (endValue < 0 || endValue >= totalValues) {
                observerError.store(true);
            }
        }
        Coroutine.Schedule();
    }
}

async function smallStableMPMCTest002(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(5);
    let produced = new AtomicInt(0);
    let observerError = new AtomicBoolean(false);

    let producer1 = producer(queue, 0, valuesPerProducer, produced);
    let producer2 = producer(queue, valuesPerProducer, valuesPerProducer, produced);
    let consumer1 = consumer(queue, totalValues / consumerCount);
    let consumer2 = consumer(queue, totalValues / consumerCount);
    let observer = observeQueue(queue, produced, observerError);

    let consumed = await Promise.all([consumer1, consumer2]);
    await Promise.all([producer1, producer2, observer]);

    let counts = new FixedArray<int>(totalValues);
    for (let i = 0; i < consumed.length; i++) {
        for (let j = 0; j < totalValues; j++) {
            counts[j] += consumed[i][j];
        }
    }
    for (let i = 0; i < totalValues; i++) {
        arktest.assertEQ(counts[i], 1);
    }
    arktest.assertFalse(observerError.load());
    arktest.assertTrue(queue.isEmpty());
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueuePopStressTest');

    testSuite.addAsyncTest('ThreadsPopStressTest001', threadsPopStressTest001);
    testSuite.addAsyncTest('SmallStableMPMCTest002', smallStableMPMCTest002);

    return testSuite.run();
}