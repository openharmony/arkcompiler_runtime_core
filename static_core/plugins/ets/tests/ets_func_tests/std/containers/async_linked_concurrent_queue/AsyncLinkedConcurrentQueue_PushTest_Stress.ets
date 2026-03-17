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

async function pushAndMark(queue: containers.stackless.LinkedConcurrentQueue<QueueValue>, value: int,
    producerFinished: AtomicBoolean): Promise<void> {
    await queue.push(new QueueValue(value));
    producerFinished.store(true);
}

async function threadsPushStressTest001(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(stressNum);
    let promises = Array.create<Promise<void>>(0, Promise.resolve());
    for (let i = 0; i < stressNum; i++) {
        promises.push(queue.push(new QueueValue(i)));
    }
    await Promise.all(promises);
    arktest.assertEQ(queue.size, stressNum);
    arktest.assertEQ(queue.getFirst()!.value, 0);
}

async function pushBlocksWhenFullTest002(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(1);
    let producerFinished = new AtomicBoolean(false);

    await queue.push(new QueueValue(0));
    let producer = pushAndMark(queue, 1, producerFinished);

    Coroutine.Schedule();
    arktest.assertFalse(producerFinished.load());
    arktest.assertEQ((await queue.pop()).value, 0);

    await producer;
    arktest.assertTrue(producerFinished.load());
    arktest.assertEQ(queue.size, 1);
    arktest.assertEQ(queue.getFirst()!.value, 1);
    arktest.assertEQ(queue.getEnd()!.value, 1);
    arktest.assertEQ((await queue.pop()).value, 1);
    arktest.assertTrue(queue.isEmpty());
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueuePushStressTest');

    testSuite.addAsyncTest('ThreadsPushStressTest001', threadsPushStressTest001);
    testSuite.addAsyncTest('PushBlocksWhenFullTest002', pushBlocksWhenFullTest002);

    return testSuite.run();
}