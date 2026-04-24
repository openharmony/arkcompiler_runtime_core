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
let queueInt: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueGetEndTest");

    ArrayBlockingQueueTestsuite.addTest("GetEndTestInt001", () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            let el = (random() * 10000).toInt();
            queueInt.push(el);
            arktest.assertEQ(el, queueInt.getEnd());
        }
        arktest.assertEQ(queueInt.size, elementsNum);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("GetEndTestInt002", async () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let p1 = launch(queueIntGetEnddata);
        queueInt.push(elementsNum);
        await p1;
        arktest.assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("GetEndTestInt003", async () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getEnd(), elementsNum - 1);
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let p1 = launch(queueIntGetEnddata);
        queueInt.push(elementsNum);
        await p1;
        arktest.assertEQ(queueInt.size, 1);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("GetEndTestInt004", async () =>
    {
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getEnd(), elementsNum - 1);
        while (!queueInt.isEmpty()) {
            let el = queueInt.getFirst();
            arktest.assertEQ(el, queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let p1 = launch(queueIntGetEnddata);
        let a1: int = 10;
        let a2: int = 20;
        queueInt.push(a1);
        queueInt.push(a2);
        await p1;
        arktest.assertEQ(queueInt.size, 2);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("GetEndTestInt005", async () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        let p1 = launch(queueIntGetEndB1data);
        let p2 = launch(queueIntAddTwodata);
        let p3 = launch(queueIntGetEndB2data);
        let p4 = launch(queueIntAddTwodata);
        await p1;
        await p2;
        await p3;
        await p4;
        arktest.assertFalse(queueInt.isEmpty());
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("GetEndTestInt006", async () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        queueInt.push(10);
        let p1 = launch(queueIntGetEnddata);
        let p2 = launch(queueIntGetEnddata);
        queueInt.push(20);
        await p1;
        await p2;
        let p3 = launch(queueIntGetEnddata);
        await p3;
        arktest.assertEQ(queueInt.getEnd(), 20);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntGetEnddata() {
    queueInt.getEnd();
}

function queueIntGetEndB1data() {
    queueInt.getEnd();
}

function queueIntGetEndB2data() {
    queueInt.getEnd();
}

function queueIntAddTwodata() {
    queueInt.push(10);
    queueInt.push(20);
}
