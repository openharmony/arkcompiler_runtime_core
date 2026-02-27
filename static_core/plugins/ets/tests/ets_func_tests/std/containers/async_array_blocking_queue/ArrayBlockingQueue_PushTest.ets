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

let testArrayInt: Array<int> = new Array<int>(elementsNum);
let testArrayObj: Array<Object> = new Array<Object>(elementsNum);
let testArrayStr: Array<String> = new Array<String>(elementsNum);
let queueInt: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);
let queueObj: containers.stackless.ArrayBlockingQueue<Object> = new containers.stackless.ArrayBlockingQueue<Object>(200);
let queueStr: containers.stackless.ArrayBlockingQueue<String> = new containers.stackless.ArrayBlockingQueue<String>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueuePushTest");

    ArrayBlockingQueueTestsuite.addAsyncTest("SimpleQueuePushPopFrontBackSizeEmptyTest", async () =>
    {
        let arr: Array<int> = new Array<int>(elementsNum);
        for (let i: int = 0; i < arr.length; ++i) {
            let el = (random() * 10000).toInt();
            arr[i] = el;
            await queueInt.push(el);
            arktest.assertEQ(el, queueInt.getEnd());
        }
        let ind: int = 0;
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, await queueInt.pop());
            arktest.assertEQ(el, arr[ind++]);
        }
        arktest.assertEQ(ind, arr.length);
        arktest.assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("ConcurrentPushPopTestInt", async () =>
    {
        let p1 = pusherInt();
        let arrayToCheck = await popperInt();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayInt[i], arrayToCheck[i]);
        }
        await p1
        arktest.assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("ConcurrentPushPopTestObj", async () =>
    {
        let p1 = pusherObj();
        let arrayToCheck = await popperObj();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayObj[i], arrayToCheck[i]);
        }
        await p1;
        arktest.assertEQ(queueObj.size, 0);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("ConcurrentPushPopTestStr", async () =>
    {
        let p1 = pusherStr();
        let arrayToCheck = await popperStr();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayStr[i], arrayToCheck[i]);
        }
        await p1;
        arktest.assertEQ(queueStr.size, 0);
    });

    ArrayBlockingQueueTestsuite.addAsyncTest("ConcurrentPushAddTestInt", async () =>
    {
        let size = 20
        let queue = new containers.stackless.ArrayBlockingQueue<int>(size);
        let el = 1;
        let promisesPush = new Array<Promise<void>>(size / 2);
        for (let i: int = 0; i < size / 2; ++i) {
           promisesPush[i] = queue.push(el);
        }
        for (let i: int = 0; i < size / 2; ++i) {
           arktest.assertTrue(queue.add(el));
        }
        let promisesPop = new Array<Promise<int>>(size / 2);
        for (let i: int = 0; i < size / 2; ++i) {
            promisesPop[i] = queue.pop();
        }
        for (let i: int = 0; i < size / 2; i++) {
            await promisesPush[i];
            await promisesPop[i];
        }
        arktest.assertEQ(queue.size, size / 2)
    });

    ArrayBlockingQueueTestsuite.addAsyncTest("ConcurrentPushPopTestInt", async () =>
    {
        let size = 20
        let queue = new containers.stackless.ArrayBlockingQueue<int>(size);
        let el = 1;
        let promisesPush = new Array<Promise<void>>(size);
        for (let i: int = 0; i < size; ++i) {
           promisesPush[i] = queue.push(el);
        }
        let promisesPop = new Array<Promise<int>>(size);
        for (let i: int = 0; i < size; ++i) {
            promisesPop[i] = queue.pop();
        }
        for (let i: int = 0; i < size; i++) {
            await promisesPush[i];
            await promisesPop[i];
        }
        arktest.assertEQ(queue.size, 0)
    });

    return ArrayBlockingQueueTestsuite.run();
}

async function pusherInt(): Promise<void> {
    for (let i = 0; i < elementsNum; i++) {
        let el = (random() * 10000).toInt();
        testArrayInt[i] = el;
        await queueInt.push(el);
    }
}

async function pusherObj() {
    for (let i = 0; i < elementsNum; i++) {
        let el = new Object();
        testArrayObj[i] = el;
        await queueObj.push(el);
    }
}

async function pusherStr() {
    for (let i = 0; i < elementsNum; i++) {
        if (i == 0) {
            let el = String("abacaba");
            testArrayStr[i] = el;
            await queueStr.push(el);
        } else {
            let el = testArrayStr[i - 1] + "aba";
            testArrayStr[i] = el;
            await queueStr.push(el);
        }
    }
}

async function popperInt(): Promise<Array<int>> {
    let arr = new Array<int>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        arr[i] = await queueInt.pop();
    }
    return arr;
}

async function popperObj(): Promise<Array<Object>> {
    let arr = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        arr[i] = await queueObj.pop();
    }
    return arr;
}

async function popperStr(): Promise<Array<String>> {
    let arr = new Array<String>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        arr[i] = await queueStr.pop();
    }
    return arr;
}
