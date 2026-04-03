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
const testNum: int = 20;

class QueueValue {
    constructor(value: int) {
        this.value = value;
    }

    public value: int;
}

function addTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < elementsNum; i++) {
        arktest.assertEQ(queue.add(new QueueValue(i)), true);
    }
    arktest.assertEQ(queue.isEmpty(), false);
    arktest.assertEQ(queue.size, elementsNum);
}

function addTest002(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        arktest.assertEQ(queue.add(new QueueValue(i)), true);
    }
    arktest.assertEQ(queue.add(new QueueValue(testNum)), false);
    arktest.assertEQ(queue.size, elementsNum);
}

async function addAndPushAndPollTest003(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < 10; i++) {
        queue.add(new QueueValue(i));
    }
    for (let i = 10; i < 20; i++) {
        await queue.push(new QueueValue(i));
    }
    for (let i = 0; i < 20; i++) {
        arktest.assertEQ(queue.size, 20 - i);
        arktest.assertEQ(queue.poll()!.value, i);
    }
}

async function addAndPushAndPopTest004(): Promise<void> {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    for (let i = 0; i < 10; i++) {
        queue.add(new QueueValue(i));
    }
    for (let i = 10; i < 20; i++) {
        await queue.push(new QueueValue(i));
    }
    for (let i = 0; i < 20; i++) {
        arktest.assertEQ(queue.size, 20 - i);
        arktest.assertEQ((await queue.pop()).value, i);
    }
    arktest.assertEQ(queue.isEmpty(), true);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueAddTest');

    testSuite.addTest('AddTest001', addTest001);
    testSuite.addTest('AddTest002', addTest002);
    testSuite.addAsyncTest('AddAndPushAndPollTest003', addAndPushAndPollTest003);
    testSuite.addAsyncTest('AddAndPushAndPopTest004', addAndPushAndPopTest004);

    return testSuite.run();
}