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
const values: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

class QueueValue {
    constructor(value: int) {
        this.value = value;
    }

    public value: int;
}

function expectRangeError(error: Error): boolean {
    return error instanceof RangeError;
}

function constructorTest001(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>();
    arktest.assertEQ(queue.isEmpty(), true);
    arktest.assertEQ(queue.size, 0);
    arktest.assertEQ(queue.capacity, Int.MAX_VALUE);
}

function constructorTest002(): void {
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(elementsNum);
    arktest.assertEQ(queue.isEmpty(), true);
    arktest.assertEQ(queue.size, 0);
    arktest.assertEQ(queue.capacity, elementsNum);
}

async function constructorTest003(): Promise<void> {
    let array = Array.create<QueueValue>(0, new QueueValue(0));
    for (let i = 0; i < values.length; i++) {
        array.push(new QueueValue(values[i]));
    }
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(array);
    for (let i = 0; i < values.length; i++) {
        arktest.assertEQ((await queue.pop()).value, values[i]);
    }
    arktest.assertEQ(queue.capacity, Int.MAX_VALUE);
}

async function constructorTest004(): Promise<void> {
    let array = Array.create<QueueValue>(0, new QueueValue(0));
    for (let i = 0; i < values.length; i++) {
        array.push(new QueueValue(values[i]));
    }
    let queue: containers.stackless.LinkedConcurrentQueue<QueueValue> =
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(elementsNum, array);
    for (let i = 0; i < values.length; i++) {
        arktest.assertEQ((await queue.pop()).value, values[i]);
    }
    arktest.assertEQ(queue.capacity, elementsNum);
}

function constructorTest005(): void {
    arktest.expectThrow(() => {
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(0);
    }, expectRangeError);
    arktest.expectThrow(() => {
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(-1);
    }, expectRangeError);
}

function constructorTest006(): void {
    let array = Array.create<QueueValue>(0, new QueueValue(0));
    for (let i = 0; i <= elementsNum; i++) {
        array.push(new QueueValue(i));
    }
    arktest.expectThrow(() => {
        new containers.stackless.LinkedConcurrentQueue<QueueValue>(elementsNum, array);
    }, expectRangeError);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('LinkedConcurrentQueueConstructorTest');

    testSuite.addTest('ConstructorTest001', constructorTest001);
    testSuite.addTest('ConstructorTest002', constructorTest002);
    testSuite.addAsyncTest('ConstructorTest003', constructorTest003);
    testSuite.addAsyncTest('ConstructorTest004', constructorTest004);
    testSuite.addTest('ConstructorTest005', constructorTest005);
    testSuite.addTest('ConstructorTest006', constructorTest006);

    return testSuite.run();
}