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
let b1: int | null = 0;
let b2: int | null = 0;
let backNum: int | null = 0;

let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new ArkTestsuite("ArrayBlockingQueueGetEndTest");

    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt001", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            let el = (random() * 10000) as int;
            queueInt.push(el);
            assertEQ(el, queueInt.getEnd());
        }
        assertEQ(queueInt.size, elementsNum);
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt002", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        assertEQ(queueInt.size, 0);
        let p1 = launch queueIntGetEnddata();
        queueInt.push(elementsNum);
        await p1;
        assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt003", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        assertEQ(queueInt.size, elementsNum);
        assertEQ(queueInt.getEnd(), elementsNum - 1);
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        assertEQ(queueInt.size, 0);
        let p1 = launch queueIntGetEnddata();
        queueInt.push(elementsNum);
        await p1;
        assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt004", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        assertEQ(queueInt.size, elementsNum);
        assertEQ(queueInt.getEnd(), elementsNum - 1);
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            assertEQ(el, queueInt.pop());
        }
        assertEQ(queueInt.size, 0);
        let p1 = launch queueIntGetEnddata();
        let a1: int = 10;
        let a2: int = 20;
        queueInt.push(a1);
        queueInt.push(a2);
        await p1;
        assertEQ(queueInt.size, 2);
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt005", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        let p1 = launch queueIntGetEndB1data();
        let p2 = launch queueIntAddTwodata();
        let p3 = launch queueIntGetEndB2data();
        let p4 = launch queueIntAddTwodata();
        await p1;
        await p2;
        await p3;
        await p4;
        assertFalse(queueInt.isEmpty());
    });
    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt006", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        queueInt.push(10);
        let p1 = launch queueIntGetEnddata();
        let p2 = launch queueIntGetEnddata();
        queueInt.push(20);
        await p1;
        await p2;
        let p3 = launch queueIntGetEnddata();
        await p3;
        assertEQ(queueInt.getEnd(), 20);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntGetEnddata() {
    backNum = queueInt.getEnd();
}

function queueIntGetEndB1data() {
    b1 = queueInt.getEnd();
}

function queueIntGetEndB2data() {
    b2 = queueInt.getEnd();
}

function queueIntAddTwodata() {
    queueInt.push(10);
    queueInt.push(20);
}
