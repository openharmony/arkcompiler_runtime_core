/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
let queueInt: BlockingQueue<int> = new ArrayBlockingQueue<int>(100);
let queueObj: BlockingQueue<Object> = new LinkedBlockingQueue<Object>(100);
let queueStr: BlockingQueue<String> = new LinkedBlockingQueue<String>(100);

function wait(): int {
    let i = 1
    for (let j = 0; j < 3000; j++) {
        i += j
    }
    return i
}

function pusherInt() {
    for (let i = 0; i < elementsNum; i++) {
        wait();
        let el = (random() * 10000) as int;
        testArrayInt[i] = el;
        queueInt.push(el);
    }
}

function pusherObj() {
    for (let i = 0; i < elementsNum; i++) {
        wait();
        let el = new Object();
        testArrayObj[i] = el;
        queueObj.push(el);
    }
}

function pusherStr() {
    for (let i = 0; i < elementsNum; i++) {
        wait();
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
        if (el === null) {
            continue;
        }
        arr[i] = queueInt.pop();
        assertEQ(el, arr[i]);
    }
    return arr;
}

function popperObj(): Array<Object> {
    let arr = new Array<Object>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueObj.getFirst();
        if (el === null) {
            continue;
        }
        arr[i] = queueObj.pop();
        assertEQ(el, arr[i]);
    }
    return arr;
}

function popperStr(): Array<String> {
    let arr = new Array<String>(elementsNum);
    for (let i = 0; i < elementsNum; i++) {
        let el = queueStr.getFirst();
        if (el === null) {
            continue;
        }
        arr[i] = queueStr.pop();
        assertEQ(el, arr[i]);
    }
    return arr;
}

function concurrentPushPopTestInt() {
    let p1 = launch pusherInt();
    await p1;
    let arrayToCheck = popperInt();
    for (let i = 0; i < elementsNum; i++) {
        assertEQ(testArrayInt[i], arrayToCheck[i]);
    }
    assertEQ(queueInt.size, 0);
}

function concurrentPushPopTestObj() {
    let p1 = launch pusherObj();
    await p1;
    let arrayToCheck = popperObj();
    for (let i = 0; i < elementsNum; i++) {
        assertEQ(testArrayObj[i], arrayToCheck[i]);
    }
    assertEQ(queueObj.size, 0);
}

function concurrentPushPopTestStr() {
    let p1 = launch pusherStr();
    await p1;
    let arrayToCheck = popperStr();
    for (let i = 0; i < elementsNum; i++) {
        assertEQ(testArrayStr[i], arrayToCheck[i]);
    }
    assertEQ(queueStr.size, 0);
}

function simpleQueuePushPopFrontBackSizeEmptyTest() {
    let arr: Array<int> = new Array<int>(elementsNum);
    for (let i: int = 0; i < arr.length; ++i) {
        let el = (random() * 10000) as int;
        arr[i] = el;
        queueInt.push(el);
        assertEQ(el, (queueInt as ArrayBlockingQueue<int>).getEnd());
    }
    let ind: int = 0;
    while (!queueInt.isEmpty()) {
        let el = queueInt.getFirst();
        assertEQ(el, queueInt.pop());
        assertEQ(el, arr[ind++]);
    }
    assertEQ(ind, arr.length);
    assertEQ(queueInt.size, 0);
}

function main() {
    simpleQueuePushPopFrontBackSizeEmptyTest();
    concurrentPushPopTestInt(); // Concurrent test for Int type
    concurrentPushPopTestObj(); // Concurrent test for Object type
    concurrentPushPopTestStr(); // Concurrent test for String type
}
