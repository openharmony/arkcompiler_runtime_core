/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
let f1: int | null = 0;

let testArrayInt: Array<int> = new Array<int>(elementsNum);
let testArrayObj: Array<Object> = new Array<Object>(elementsNum);
let testArrayStr: Array<String> = new Array<String>(elementsNum);
let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
let queueObj: ArrayBlockingQueue<Object> = new ArrayBlockingQueue<Object>(200);
let queueStr: ArrayBlockingQueue<String> = new ArrayBlockingQueue<String>(200);
let queueIntPopN: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new ArkTestsuite("ArrayBlockingQueuePopTest");

    ArrayBlockingQueueTestsuite.addTest("ConcurrentPushPopTestStr", () =>
    {
        let p1 = launch pusherStr();
        let arrayToCheck = popperStr();
        await p1;
        for (let i = 0; i < elementsNum; i++) {
            assertEQ(testArrayStr[i], arrayToCheck[i]);
        }
        assertEQ(queueStr.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt001", () =>
    {
        let queueIntPop1: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        assertEQ(queueIntPop1.size, 0);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPop1.push(i);
        }
        assertEQ(queueIntPop1.getFirst(), 0);
        queueIntPop1.pop();
        assertEQ(queueIntPop1.size, elementsNum - 1);
        assertTrue(queueIntPop1.getFirst() != 0);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt002", () =>
    {
        let queueIntPop2: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPop2.push(i);
        }
        let num: int | null = queueIntPop2.getFirst();
        assertEQ(num, queueIntPop2.pop());
        assertEQ(queueIntPop2.size, elementsNum - 1);
        while (!queueIntPop2.isEmpty()) {
            let el = queueIntPop2.getFirst();
            assertEQ(el, queueIntPop2.pop());
        }
        let p1 = launch ArrayBlockingQueuePop(queueIntPop2);
        let i = queueIntPop2.pop();
        await p1;
        assertEQ(i, 10);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt003", () =>
    {
        let p1 = launch queueIntPopNdata();
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPopN.push(i);
        }
        await p1;
        assertEQ(queueIntPopN.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt004", () =>
    {
        let queueIntPop: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        assertEQ(queueIntPop.size, 0);
        for (let i: int = 0; i < elementsNum; ++i) {
            let el = (random() * 10000) as int;
            queueIntPop.push(el);
        }
        assertEQ(queueIntPop.size, elementsNum);
        let j: int = 1;
        for (let i = 0; i < elementsNum; i++) {
            queueIntPop.pop();
            assertEQ(queueIntPop.size, elementsNum - j);
            j++;
        }
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt005", () =>
    {
        while (!queueIntPopN.isEmpty()) {
            queueIntPopN.pop();
        }
        let promises = new Array<Promise<void>>(100);
        for (let i: int = 0; i < elementsNum; ++i) {
           promises[i] = launch queueIntPopdata(); 
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntPopN.push(i);
        }
        for (let i: int = 0; i < 100; i++) {
            await promises[i];
        }
        assertEQ(queueIntPopN.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("PopTestInt006", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        let p1 = launch queueIntFrontF1data();
        let p2 = launch queueIntPopFdata();
        await p1;
        await p2;
        assertEQ(queueInt.size, elementsNum - 1);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function pusherInt() {
    for (let i = 0; i < elementsNum; i++) {
        let el = (random() * 10000) as int;
        testArrayInt[i] = el;
        queueInt.push(el);
    }
}

function pusherObj() {
    for (let i = 0; i < elementsNum; i++) {
        let el = new Object();
        testArrayObj[i] = el;
        queueObj.push(el);
    }
}

function pusherStr() {
    for (let i = 0; i < elementsNum; i++) {
        if (i == 0) {
            let el = String("abacaba");
            testArrayStr[i] = el;
            queueStr.push(el);
        } else {
            let el = testArrayStr[i - 1] + "aba";
            testArrayStr[i] = el;
            queueStr.push(el);
        }
    }
}

function popperInt(): Array<int> {
    let arr = new Array<int>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueInt.getFirst();
        arr[i] = queueInt.pop();
        assertEQ(el, arr[i]);
    }
    return arr;
}

function popperObj(): Array<Object> {
    let arr = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueObj.getFirst();
        arr[i] = queueObj.pop();
        assertEQ(el, arr[i]);
    }
    return arr;
}

function popperStr(): Array<String> {
    let arr = new Array<String>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        arr[i] = queueStr.pop();
    }
    return arr;
}

function queueIntPopdata() {
    queueIntPopN.pop();
}

function queueIntPopFdata() {
    queueInt.pop();
}

function ArrayBlockingQueuePop (queueIntPop2: ArrayBlockingQueue<int>) {
    queueIntPop2.push(10);
}

function queueIntPopNdata () {
    for (let i: int = 0; i < elementsNum; ++i) {
        queueIntPopN.pop();
    }
}

function queueIntFrontF1data() {
    f1 = queueInt.getFirst();
}
