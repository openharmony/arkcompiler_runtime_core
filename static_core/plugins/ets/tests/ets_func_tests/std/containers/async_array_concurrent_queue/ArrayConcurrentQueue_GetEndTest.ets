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
let queueInt: containers.stackless.ArrayConcurrentQueue<int> = new containers.stackless.ArrayConcurrentQueue<int>(200);

function main(): int {
    let arrayConcurrentQueueTestsuite = new arktest.ArkTestsuite('ArrayConcurrentQueueGetEndTest');

    arrayConcurrentQueueTestsuite.addAsyncTest('GetEndTestInt001', async () => {
        for (let i: int = 0; i < elementsNum; ++i) {
            let queueItem = (random() * 10000).toInt();
            await queueInt.push(queueItem);
            arktest.assertEQ(queueItem, queueInt.getEnd());
        }
        arktest.assertEQ(queueInt.size, elementsNum);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetEndTestInt002', async () => {
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let job1 = launch(queueIntGetEndData);
        await queueInt.push(elementsNum);
        job1.Await();
        arktest.assertEQ(queueInt.size, 1);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetEndTestInt003', async () => {
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueInt.push(i);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getEnd(), elementsNum - 1);
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let job1 = launch(queueIntGetEndData);
        await queueInt.push(elementsNum);
        job1.Await();
        arktest.assertEQ(queueInt.size, 1);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetEndTestInt004', async () => {
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            await queueInt.push(i);
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getEnd(), elementsNum - 1);
        while (!queueInt.isEmpty()) {
            let queueItem = queueInt.getFirst();
            arktest.assertEQ(queueItem, await queueInt.pop());
        }
        arktest.assertEQ(queueInt.size, 0);
        let job1 = launch(queueIntGetEndData);
        let firstValue: int = 10;
        let secondValue: int = 20;
        await queueInt.push(firstValue);
        await queueInt.push(secondValue);
        job1.Await();
        arktest.assertEQ(queueInt.size, 2);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetEndTestInt005', async () => {
        let job1 = launch(queueIntGetEndBatch1Data);
        let job2 = launch(queueIntAddTwoData);
        let job3 = launch(queueIntGetEndBatch2Data);
        let job4 = launch(queueIntAddTwoData);
        job1.Await();
        job2.Await();
        job3.Await();
        job4.Await();
        arktest.assertFalse(queueInt.isEmpty());
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('GetEndTestInt006', async () => {
        while (!queueInt.isEmpty()) {
            await queueInt.pop();
        }
        queueInt.push(10);
        let job1 = launch(queueIntGetEndData);
        let job2 = launch(queueIntGetEndData);
        queueInt.push(20);
        job1.Await();
        job2.Await();
        let job3 = launch(queueIntGetEndData);
        job3.Await();
        arktest.assertEQ(queueInt.getEnd(), 20);
    });

    return arrayConcurrentQueueTestsuite.run();
}

function queueIntGetEndData() {
    queueInt.getEnd();
}

function queueIntGetEndBatch1Data() {
    queueInt.getEnd();
}

function queueIntGetEndBatch2Data() {
    queueInt.getEnd();
}

function queueIntAddTwoData() {
    queueInt.push(10);
    queueInt.push(20);
}
