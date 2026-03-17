/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

const elementsNum: int = 100;

let testArrayInt: int[] = new Array<int>(elementsNum);
let testArrayObj: Object[] = new Array<Object>(elementsNum);
let testArrayStr: String[] = new Array<String>(elementsNum);
let queueInt: containers.stackless.ArrayConcurrentQueue<int> = new containers.stackless.ArrayConcurrentQueue<int>(200);
let queueObj: containers.stackless.ArrayConcurrentQueue<Object> =
    new containers.stackless.ArrayConcurrentQueue<Object>(200);
let queueStr: containers.stackless.ArrayConcurrentQueue<String> =
    new containers.stackless.ArrayConcurrentQueue<String>(200);
let queueIntPopQueue: containers.stackless.ArrayConcurrentQueue<int> =
    new containers.stackless.ArrayConcurrentQueue<int>(200);

function main(): int {
    let arrayConcurrentQueueTestsuite = new arktest.ArkTestsuite('ArrayConcurrentQueuePopTest');

    arrayConcurrentQueueTestsuite.addAsyncTest('ConcurrentPushPopTestStr', async () => {
        let pushPromise = pusherStr();
        let arrayToCheck = await popperStr();
        await pushPromise;
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayStr[i], arrayToCheck[i]);
        }
        arktest.assertEQ(queueStr.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('PopTestInt001', async () => {
        let queueIntPop1: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        arktest.assertEQ(queueIntPop1.size, 0);
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueIntPop1.push(i);
        }
        arktest.assertEQ(queueIntPop1.getFirst(), 0);
        await queueIntPop1.pop();
        arktest.assertEQ(queueIntPop1.size, elementsNum - 1);
        arktest.assertEQ(queueIntPop1.getFirst(), 1);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('PopTestInt002', async () => {
        let queueIntPop2: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueIntPop2.push(i);
        }
        let queueItem = queueIntPop2.getFirst();
        arktest.assertEQ(queueItem, await queueIntPop2.pop());
        arktest.assertEQ(queueIntPop2.size, elementsNum - 1);
        while (!queueIntPop2.isEmpty()) {
            let currentItem = queueIntPop2.getFirst();
            arktest.assertEQ(currentItem, await queueIntPop2.pop());
        }
        let pushPromise = arrayConcurrentQueuePush(queueIntPop2);
        let result = await queueIntPop2.pop();
        await pushPromise;
        arktest.assertEQ(result, 10);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('PopTestInt003', async () => {
        let popPromise = queueIntPopBatchData();
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueIntPopQueue.push(i);
        }
        await popPromise;
        arktest.assertEQ(queueIntPopQueue.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('PopTestInt004', async () => {
        let queueIntPop: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        arktest.assertEQ(queueIntPop.size, 0);
        for (let i: int = 0; i < elementsNum; ++i) {
            let queueItem = (random() * 10000).toInt();
            await queueIntPop.push(queueItem);
        }
        arktest.assertEQ(queueIntPop.size, elementsNum);
        let currentSizeOffset: int = 1;
        for (let i = 0; i < elementsNum; i++) {
            await queueIntPop.pop();
            arktest.assertEQ(queueIntPop.size, elementsNum - currentSizeOffset);
            currentSizeOffset++;
        }
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('PopTestInt005', async () => {
        while (!queueIntPopQueue.isEmpty()) {
            await queueIntPopQueue.pop();
        }
        let promises: Promise<void>[] = new Array<Promise<void>>(100);
        for (let i: int = 0; i < elementsNum; ++i) {
            promises[i] = queueIntPopData();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPopQueue.push(i);
        }
        for (let i: int = 0; i < 100; i++) {
            await promises[i];
        }
        arktest.assertEQ(queueIntPopQueue.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('PopTestInt006', async () => {
        while (!queueInt.isEmpty()) {
            await queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueInt.push(i);
        }
        let job1 = launch(queueIntFrontData);
        let popPromise = queueIntPopFrontData();
        job1.Await();
        await popPromise;
        arktest.assertEQ(queueInt.size, elementsNum - 1);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('ConcurrentPopPollInt', async () => {
        let queueSize = 20;
        let queue: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(queueSize);
        let queueItem = 1;
        for (let i: int = 0; i < queueSize; ++i) {
            await queue.push(queueItem);
        }
        let promisesPop: Promise<int>[] = new Array<Promise<int>>(queueSize / 2);
        for (let i: int = 0; i < queueSize / 2; ++i) {
            promisesPop[i] = queue.pop();
        }
        for (let i: int = 0; i < queueSize / 2; ++i) {
            arktest.assertEQ(queue.poll(), queueItem);
        }
        for (let i: int = 0; i < queueSize / 2; i++) {
            await promisesPop[i];
        }
        arktest.assertEQ(queue.size, 0);
    });

    return arrayConcurrentQueueTestsuite.run();
}

async function pusherInt() {
    for (let i = 0; i < elementsNum; i++) {
        let queueItem = (random() * 10000).toInt();
        testArrayInt[i] = queueItem;
        queueInt.push(queueItem);
    }
}

async function pusherObj() {
    for (let i = 0; i < elementsNum; i++) {
        let queueItem = new Object();
        testArrayObj[i] = queueItem;
        await queueObj.push(queueItem);
    }
}

async function pusherStr() {
    for (let i = 0; i < elementsNum; i++) {
        if (i == 0) {
            let queueItem = String('abacaba');
            testArrayStr[i] = queueItem;
            await queueStr.push(queueItem);
        } else {
            let queueItem = testArrayStr[i - 1] + 'aba';
            testArrayStr[i] = queueItem;
            await queueStr.push(queueItem);
        }
    }
}

async function popperInt(): Promise<int[]> {
    let result: int[] = new Array<int>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let queueItem = queueInt.getFirst();
        result[i] = await queueInt.pop();
        arktest.assertEQ(queueItem, result[i]);
    }
    return result;
}

async function popperObj(): Promise<Object[]> {
    let result: Object[] = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let queueItem = queueObj.getFirst();
        result[i] = await queueObj.pop();
        arktest.assertEQ(queueItem, result[i]);
    }
    return result;
}

async function popperStr(): Promise<String[]> {
    let result: String[] = new Array<String>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        result[i] = await queueStr.pop();
    }
    return result;
}

async function queueIntPopData() {
    await queueIntPopQueue.pop();
}

async function queueIntPopFrontData() {
    await queueInt.pop();
}

async function arrayConcurrentQueuePush(queueIntPop2: containers.stackless.ArrayConcurrentQueue<int>) {
    await queueIntPop2.push(10);
}

async function queueIntPopBatchData() {
    for (let i: int = 0; i < elementsNum; ++i) {
        await queueIntPopQueue.pop();
    }
}

function queueIntFrontData() {
    queueInt.getFirst();
}
