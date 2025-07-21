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

import { launch } from "std/concurrency"

const elementsNum: int = 10;
const stressNum: int = 100;
let b1: int | null = 0;
let f1: int | null = 0;
let f2: int | null = 0;
let backNum: int | null = 0;

let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new ArkTestsuite("ArrayBlockingQueueGetFirstTest");

    ArrayBlockingQueueTestsuite.addTest("GetFirstTest001", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        for (let i = 0; i < elementsNum; i++) {
            assertEQ(queueInt.add(i), true);
        }
        let res = queueInt.getFirst();
        assertEQ(res, 0);
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.size, elementsNum);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTest002", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        for (let i = 0; i < elementsNum; i++) {
            assertEQ(queueInt.add(i), true);
        }
        queueInt.pop();
        let res = queueInt.getFirst();
        assertEQ(res, 1);
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.size, elementsNum - 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstAndSizeAndPopAndPollTest009", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        assertEQ(queueInt.getFirst(), 0);
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.size, 20 - i);
            queueInt.pop();
        }
        assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.size, 10 - i);
            queueInt.poll();
        }
        assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstAndIsEmptyAndPopAndPollTest010", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        assertEQ(queueInt.getFirst(), 0);
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            queueInt.pop();
        }
        assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            queueInt.poll();
        }
        assertEQ(queueInt.isEmpty(), true);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstStressTest005", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < stressNum; i++) {
            assertEQ(queueInt.getFirst(), i);
            queueInt.pop();
        }
        assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt001", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        let frontNum = 0;
        for (let i: int = 0; i < elementsNum; ++i) {
            let el = (random() * 10000) as int;
            queueInt.push(el);
            if (i == 0) {
                frontNum = el;
            }
        }
        assertEQ(queueInt.getFirst(), frontNum);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt002", () =>
    {
        backNum = 0;
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        let p1 = launch<void, () => void>(queueIntGetFirstdata);
        let frontNum: int = 10;
        queueInt.push(frontNum);
        p1.Await();
        assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt003", () =>
    {
        backNum = 0;
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        assertEQ(queueInt.size, elementsNum);
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        assertEQ(queueInt.size, 0);
        let p1 = launch<void, () => void>(queueIntGetFirstdata);
        let frontNum: int = 10;
        queueInt.push(frontNum);
        p1.Await();
        assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt004", () =>
    {
        b1 = 0;
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        let p1 = launch<void, () => void>(queueIntGetFirstF1data);
        queueInt.push(10);
        queueInt.push(20);
        p1.Await();
        let p2 = launch<void, () => void>(queueIntGetEndB1data);
        p2.Await();
        assertEQ(queueInt.size, 2);
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt005", () =>
    {
        f1 = 0;
        f2 = 0;
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        assertTrue(queueInt.isEmpty());
        let p1 = launch<void, () => void>(queueIntAddTwodata);
        let p2 = launch<void, () => void>(queueIntGetFirstF1data);
        let p3 = launch<void, () => void>(queueIntGetFirstF2data);
        p1.Await();
        p2.Await();
        p3.Await();
        assertTrue(f1 == 10 || f1 == null);
        assertTrue(f2 == 10 || f2 == null);
        assertFalse(queueInt.isEmpty());
    });
    ArrayBlockingQueueTestsuite.addTest("GetFirstTestInt006", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        queueInt.push(10);
        queueInt.push(30);
        let p1 = launch<void, () => void>(queueIntGetFirstdata);
        queueInt.push(20);
        p1.Await();
        let p2 = launch<void, () => void>(queueIntGetFirstdata);
        p2.Await();
        assertEQ(queueInt.getFirst(), 10);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntGetEnddata() {
    backNum = queueInt.getEnd();
}

function queueIntGetEndB1data() {
    b1 = queueInt.getEnd();
}

function queueIntGetFirstF1data() {
    f1 = queueInt.getFirst();
}

function queueIntGetFirstF2data() {
    f2 = queueInt.getFirst();
}

function queueIntAddTwodata() {
    queueInt.push(10);
    queueInt.push(20);
}

function queueIntGetFirstdata() {
    queueInt.getFirst();
}
