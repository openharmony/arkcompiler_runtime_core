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

const elementsNum: int = 10;
const stressNum: int = 100;
let firstValue1: int | undefined = 0;
let firstValue2: int | undefined = 0;

let queueInt: containers.stackless.ArrayConcurrentQueue<int> = new containers.stackless.ArrayConcurrentQueue<int>(200);

function main(): int {
    let arrayConcurrentQueueTestsuite = new arktest.ArkTestsuite('ArrayConcurrentQueueGetFirstTest');

    arrayConcurrentQueueTestsuite.addTest('GetFirstTest001', () => {
        let queueInt: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertTrue(queueInt.add(i));
        }
        let result = queueInt.getFirst();
        arktest.assertEQ(result, 0);
        arktest.assertFalse(queueInt.isEmpty());
        arktest.assertEQ(queueInt.size, elementsNum);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstTest002', async () => {
        let queueInt: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertTrue(queueInt.add(i));
        }
        await queueInt.pop();
        let result = queueInt.getFirst();
        arktest.assertEQ(result, 1);
        arktest.assertFalse(queueInt.isEmpty());
        arktest.assertEQ(queueInt.size, elementsNum - 1);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstAndSizeAndPopAndPollTest009', async () => {
        let queueInt: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 10; i < 20; i++) {
            await queueInt.push(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.size, 20 - i);
            await queueInt.pop();
        }
        arktest.assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.size, 10 - i);
            queueInt.poll();
        }
        arktest.assertEQ(queueInt.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstAndIsEmptyAndPopAndPollTest010', async () => {
        let queueInt: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 10; i < 20; i++) {
            await queueInt.push(i);
        }
        arktest.assertFalse(queueInt.isEmpty());
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            await queueInt.pop();
        }
        arktest.assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            queueInt.poll();
        }
        arktest.assertTrue(queueInt.isEmpty());
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstStressTest005', async () => {
        let queueInt: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < stressNum; i++) {
            arktest.assertEQ(queueInt.getFirst(), i);
            await queueInt.pop();
        }
        arktest.assertEQ(queueInt.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstTestInt001', async () => {
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
        }
        let frontNum = 0;
        for (let i: int = 0; i < elementsNum; ++i) {
            let queueItem = (random() * 10000).toInt();
            await queueInt.push(queueItem);
            if (i == 0) {
                frontNum = queueItem;
            }
        }
        arktest.assertEQ(queueInt.getFirst(), frontNum);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstTestInt002', async () => {
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
        }
        let job1 = launch(queueIntGetFirstData);
        let frontNum: int = 10;
        await queueInt.push(frontNum);
        job1.Await();
        arktest.assertEQ(queueInt.size, 1);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstTestInt003', async () => {
        while (!queueInt.isEmpty()) {
            await queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueInt.push(i);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let job1 = launch(queueIntGetFirstData);
        let frontNum: int = 10;
        await queueInt.push(frontNum);
        job1.Await();
        arktest.assertEQ(queueInt.size, 1);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstTestInt004', async () => {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        let job1 = launch(queueIntGetFirstValue1Data);
        await queueInt.push(10);
        await queueInt.push(20);
        job1.Await();
        let job2 = launch(queueIntGetEndBatch1Data);
        job2.Await();
        arktest.assertEQ(queueInt.size, 2);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstTestInt005', async () => {
        firstValue1 = 0;
        firstValue2 = 0;
        while (!queueInt.isEmpty()) {
            await queueInt.pop();
        }
        arktest.assertTrue(queueInt.isEmpty());
        let promise = queueIntAddTwoData();
        let job1 = launch(queueIntGetFirstValue1Data);
        let job2 = launch(queueIntGetFirstValue2Data);
        await promise;
        job1.Await();
        job2.Await();
        arktest.assertTrue(firstValue1 == 10 || firstValue1 == undefined);
        arktest.assertTrue(firstValue2 == 10 || firstValue2 == undefined);
        arktest.assertFalse(queueInt.isEmpty());
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetFirstTestInt006', async () => {
        while (!queueInt.isEmpty()) {
            await queueInt.pop();
        }
        await queueInt.push(10);
        await queueInt.push(30);
        let job1 = launch(queueIntGetFirstData);
        await queueInt.push(20);
        job1.Await();
        let job2 = launch(queueIntGetFirstData);
        job2.Await();
        arktest.assertEQ(queueInt.getFirst(), 10);
    });

    return arrayConcurrentQueueTestsuite.run();
}

function queueIntGetEndData() {
    queueInt.getEnd();
}

function queueIntGetEndBatch1Data() {
    queueInt.getEnd();
}

function queueIntGetFirstValue1Data() {
    firstValue1 = queueInt.getFirst();
}

function queueIntGetFirstValue2Data() {
    firstValue2 = queueInt.getFirst();
}

async function queueIntAddTwoData() {
    await queueInt.push(10);
    await queueInt.push(20);
}

function queueIntGetFirstData() {
    queueInt.getFirst();
}
