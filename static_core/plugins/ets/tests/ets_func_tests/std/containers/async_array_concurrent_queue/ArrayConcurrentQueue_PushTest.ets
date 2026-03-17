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

function main(): int {
    let arrayConcurrentQueueTestsuite = new arktest.ArkTestsuite('ArrayConcurrentQueuePushTest');

    arrayConcurrentQueueTestsuite.addAsyncTest('SimpleQueuePushPopFrontBackSizeEmptyTest', async () => {
        let array: int[] = new Array<int>(elementsNum);
        for (let i: int = 0; i < array.length; ++i) {
            let queueItem = (random() * 10000).toInt();
            array[i] = queueItem;
            await queueInt.push(queueItem);
            arktest.assertEQ(queueItem, queueInt.getEnd());
        }
        let index: int = 0;
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
            arktest.assertEQ(queueItem, array[index++]);
        }
        arktest.assertEQ(index, array.length);
        arktest.assertEQ(queueInt.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('ConcurrentPushPopTestInt', async () => {
        let pushPromise = pusherInt();
        let arrayToCheck = await popperInt();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayInt[i], arrayToCheck[i]);
        }
        await pushPromise;
        arktest.assertEQ(queueInt.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('ConcurrentPushPopTestObj', async () => {
        let pushPromise = pusherObj();
        let arrayToCheck = await popperObj();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayObj[i], arrayToCheck[i]);
        }
        await pushPromise;
        arktest.assertEQ(queueObj.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('ConcurrentPushPopTestStr', async () => {
        let pushPromise = pusherStr();
        let arrayToCheck = await popperStr();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayStr[i], arrayToCheck[i]);
        }
        await pushPromise;
        arktest.assertEQ(queueStr.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('ConcurrentPushAddTestInt', async () => {
        let queueSize = 20;
        let queue: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(queueSize);
        let queueItem = 1;
        let promisesPush: Promise<void>[] = new Array<Promise<void>>(queueSize / 2);
        for (let i: int = 0; i < queueSize / 2; ++i) {
            promisesPush[i] = queue.push(queueItem);
        }
        for (let i: int = 0; i < queueSize / 2; ++i) {
            arktest.assertTrue(queue.add(queueItem));
        }
        let promisesPop: Promise<int>[] = new Array<Promise<int>>(queueSize / 2);
        for (let i: int = 0; i < queueSize / 2; ++i) {
            promisesPop[i] = queue.pop();
        }
        for (let i: int = 0; i < queueSize / 2; i++) {
            await promisesPush[i];
            await promisesPop[i];
        }
        arktest.assertEQ(queue.size, queueSize / 2);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('ConcurrentPushPopTestInt', async () => {
        let queueSize = 20;
        let queue: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(queueSize);
        let queueItem = 1;
        let promisesPush: Promise<void>[] = new Array<Promise<void>>(queueSize);
        for (let i: int = 0; i < queueSize; ++i) {
            promisesPush[i] = queue.push(queueItem);
        }
        let promisesPop: Promise<int>[] = new Array<Promise<int>>(queueSize);
        for (let i: int = 0; i < queueSize; ++i) {
            promisesPop[i] = queue.pop();
        }
        for (let i: int = 0; i < queueSize; i++) {
            await promisesPush[i];
            await promisesPop[i];
        }
        arktest.assertEQ(queue.size, 0);
    });

    return arrayConcurrentQueueTestsuite.run();
}

async function pusherInt(): Promise<void> {
    for (let i = 0; i < elementsNum; i++) {
        let queueItem = (random() * 10000).toInt();
        testArrayInt[i] = queueItem;
        await queueInt.push(queueItem);
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
        result[i] = await queueInt.pop();
    }
    return result;
}

async function popperObj(): Promise<Object[]> {
    let result: Object[] = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        result[i] = await queueObj.pop();
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
