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

const elementsNum: int = 10;
const testNum: int = 20;
const stressNum: int = 100;

let mQueueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(elementsNum);
let sMTQueueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);

function main(): int {
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueueAddTest");

    LinkedBlockingQueueTestsuite.addTest("AddTest001", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < elementsNum; i++) {
            assertEQ(queueInt.add(i), true);
        }
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.size, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("AddTest002", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(elementsNum);
        for (let i = 0; i < elementsNum; i++) {
            assertEQ(queueInt.add(i), true);
        }
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.size, elementsNum);
        assertEQ(queueInt.add(testNum), false);
        assertEQ(queueInt.size, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("ThreadsAddTest001", () =>
    {
        while (!mQueueInt.isEmpty()) {
            mQueueInt.pop();
        }
        let promises = new Array<Promise<void>>(5);
        for (let i = 0; i < 5; i++) {
            promises[i] = launch queueAddData();
        }
        for (let i: int = 0; i < 5; i++) {
            await promises[i];
        }
        assertEQ(mQueueInt.getFirst(), testNum);
    });
    LinkedBlockingQueueTestsuite.addTest("TheadsAddAndPopTest001", () =>
    {
        while (!mQueueInt.isEmpty()) {
            mQueueInt.pop();
        }
        let promises = new Array<Promise<void>>(6);
        let p = 0;
        for (let i = 0; i < 3; i++) {
            promises[p++] = launch queueAddData();
            promises[p++] = launch queuePopData();
        }
        for (let i: int = 0; i < 6; i++) {
            await promises[i];
        }
        assertTrue(mQueueInt.size <= 3);
    });
    LinkedBlockingQueueTestsuite.addTest("TheadsAddAndGetFirstTest003", () =>
    {
        while (!mQueueInt.isEmpty()) {
            mQueueInt.pop();
        }
        let promises = new Array<Promise<void>>(10);
        let p = 0;
        for (let i = 0; i < 5; i++) {
            promises[p++] = launch queueAddData();
        }
        for (let i = 5; i < 10; i++) {
            promises[p++] = launch queueGetFirstData();
        }
        for (let i: int = 0; i < 10; i++) {
            await promises[i];
        }
        assertEQ(mQueueInt.getFirst(), testNum);
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPopAndRemainingCapacityTest001", () =>
    {
        while (!mQueueInt.isEmpty()) {
            mQueueInt.pop();
        }
        let res = queueCapacityData();
        assertEQ(res, elementsNum);
        for (let i = 0; i < 5; i++) {
            queueAddData();
        }
        res = queueRemainingCapacityData();
        assertEQ(res, elementsNum - 5);
        queuePushData();
        assertEQ(mQueueInt.size, 6);
        assertEQ(mQueueInt.getFirst(), testNum);
        for (let i = 0; i < 2; i++) {
            queuePopData();
        }
        assertEQ(mQueueInt.size, 4);
        queuePollData();
        assertEQ(mQueueInt.size, 3);
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPushAndPopTest002", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 20; i++) {
            assertEQ(queueInt.size, 20 - i);
            if (!queueInt.isEmpty()) {
                queueInt.pop();
            }
        }
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPushAndPollTest003", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 20; i++) {
            assertEQ(queueInt.size, 20 - i);
            assertEQ(queueInt.poll(), i);
        }
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPushAndGetFirstAndPollTest004", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 20; i++) {
            assertEQ(queueInt.getFirst(), i);
            assertEQ(queueInt.poll(), i);
        }
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPushAndGetFirstAndIsEnTest005", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 20; i++) {
            assertEQ(queueInt.getFirst(), i);
            assertEQ(queueInt.isEmpty(), false);
            assertEQ(queueInt.poll(), i);
        }
        assertEQ(queueInt.isEmpty(), true);
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPopAndPollTest012", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 20; i++) {
            queueInt.add(i);
        }
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            if (!queueInt.isEmpty()) {
                queueInt.pop();
            }
        }
        assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            queueInt.poll();
        }
        assertEQ(queueInt.isEmpty(), true);
    });
    LinkedBlockingQueueTestsuite.addTest("AddAndPopAndPollAndGetSizeTest014", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 20; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.size, 20 - i);
            if (!queueInt.isEmpty()) {
                queueInt.pop();
            }
        }
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.size, 10 - i);
            queueInt.poll();
        }
        assertEQ(queueInt.size, 0);
    });
    LinkedBlockingQueueTestsuite.addTest("AddStressTest001", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        assertEQ(queueInt.size, stressNum);
    });

    return LinkedBlockingQueueTestsuite.run();
}

function queueCapacityData(): int {
    return mQueueInt.capacity;
}

function queueRemainingCapacityData(): int {
    return mQueueInt.remainingCapacity();
}

function queuePushData() {
    mQueueInt.push(testNum);
}

function queueGetFirstData() {
    mQueueInt.getFirst();
}

function queuePopData() {
    if (!mQueueInt.isEmpty()) {
        mQueueInt.pop();
    }
}

function queuePollData() {
    mQueueInt.poll();
}

function queueAddData() {
    mQueueInt.add(testNum);
}
