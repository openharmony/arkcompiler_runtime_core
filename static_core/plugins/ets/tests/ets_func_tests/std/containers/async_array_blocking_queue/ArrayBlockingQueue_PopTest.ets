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
let queueIntPopN: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueuePopTest");

    ArrayBlockingQueueTestsuite.addAsyncTest("ConcurrentPushPopTestStr", async () =>
    {
        let pushPromise = pusherStr();
        let arrayToCheck = await popperStr();
        await pushPromise;
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(testArrayStr[i], arrayToCheck[i]);
        }
        arktest.assertEQ(queueStr.size, 0);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("PopTestInt001", async () =>
    {
        let queueIntPop1: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);
        arktest.assertEQ(queueIntPop1.size, 0);
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueIntPop1.push(i);
        }
        arktest.assertEQ(queueIntPop1.getFirst(), 0);
        await queueIntPop1.pop();
        arktest.assertEQ(queueIntPop1.size, elementsNum - 1);
        arktest.assertEQ(queueIntPop1.getFirst(), 1);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("PopTestInt002", async () =>
    {
        let queueIntPop2: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueIntPop2.push(i);
        }
        let num = queueIntPop2.getFirst();
        arktest.assertEQ(num, await queueIntPop2.pop());
        arktest.assertEQ(queueIntPop2.size, elementsNum - 1);
        while (!queueIntPop2.isEmpty()) {
            let el = queueIntPop2.getFirst();
            arktest.assertEQ(el, await queueIntPop2.pop());
        }
        let p = ArrayBlockingQueuePush(queueIntPop2);
        let i = await queueIntPop2.pop();
        await p;
        arktest.assertEQ(i, 10);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("PopTestInt003", async () =>
    {
        let p1 = queueIntPopNdata();
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueIntPopN.push(i);
        }
        await p1;
        arktest.assertEQ(queueIntPopN.size, 0);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("PopTestInt004", async () =>
    {
        let queueIntPop: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);
        arktest.assertEQ(queueIntPop.size, 0);
        for (let i: int = 0; i < elementsNum; ++i) {
            let el = (random() * 10000).toInt();
            await queueIntPop.push(el);
        }
        arktest.assertEQ(queueIntPop.size, elementsNum);
        let j: int = 1;
        for (let i = 0; i < elementsNum; i++) {
            await queueIntPop.pop();
            arktest.assertEQ(queueIntPop.size, elementsNum - j);
            j++;
        }
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("PopTestInt005", async () =>
    {
        while (!queueIntPopN.isEmpty()) {
            await queueIntPopN.pop();
        }
        let promises = new Array<Promise<void>>(100);
        for (let i: int = 0; i < elementsNum; ++i) {
           promises[i] = queueIntPopdata();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPopN.push(i);
        }
        for (let i: int = 0; i < 100; i++) {
            await promises[i];
        }
        arktest.assertEQ(queueIntPopN.size, 0);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("PopTestInt006", async () =>
    {
        while (!queueInt.isEmpty()) {
            await queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueInt.push(i);
        }
        let p1 = launch(queueIntFrontF1data);
        let p2 = queueIntPopFdata();
        p1.Await();
        await p2;
        arktest.assertEQ(queueInt.size, elementsNum - 1);
    });

    ArrayBlockingQueueTestsuite.addAsyncTest("ConcurrentPopPollInt", async () =>
    {
        let size = 20
        let queue = new containers.stackless.ArrayBlockingQueue<int>(size);
        let el = 1;
        for (let i: int = 0; i < size; ++i) {
           await queue.push(el);
        }
        let promisesPop = new Array<Promise<int>>(size / 2);
        for (let i: int = 0; i < size / 2; ++i) {
            promisesPop[i] = queue.pop();
        }
        for (let i: int = 0; i < size / 2; ++i) {
            arktest.assertEQ(queue.poll(), el);
        }
        for (let i: int = 0; i < size / 2; i++) {
            await promisesPop[i];
        }
        arktest.assertEQ(queue.size, 0)
    });

    return ArrayBlockingQueueTestsuite.run();
}

async function pusherInt() {
    for (let i = 0; i < elementsNum; i++) {
        let el = (random() * 10000).toInt();
        testArrayInt[i] = el;
        queueInt.push(el);
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
        let el = queueInt.getFirst();
        arr[i] = await queueInt.pop();
        arktest.assertEQ(el, arr[i]);
    }
    return arr;
}

async function popperObj(): Promise<Array<Object>> {
    let arr = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueObj.getFirst();
        arr[i] = await queueObj.pop();
        arktest.assertEQ(el, arr[i]);
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

async function queueIntPopdata() {
    await queueIntPopN.pop();
}

async function queueIntPopFdata() {
    await queueInt.pop();
}

async function ArrayBlockingQueuePush(queueIntPop2: containers.stackless.ArrayBlockingQueue<int>) {
    await queueIntPop2.push(10);
}

async function queueIntPopNdata() {
    for (let i: int = 0; i < elementsNum; ++i) {
        await queueIntPopN.pop();
    }
}

function queueIntFrontF1data() {
    queueInt.getFirst();
}
