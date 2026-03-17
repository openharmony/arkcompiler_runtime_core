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
let queueIntPopQueue: containers.stackless.ArrayConcurrentQueue<int> =
    new containers.stackless.ArrayConcurrentQueue<int>(200);

function main(): int {
    let arrayConcurrentQueueTestsuite = new arktest.ArkTestsuite('ArrayConcurrentQueueAddTest');

    arrayConcurrentQueueTestsuite.addTest('AddTestInt001', () => {
        let queueIntAdd: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            arktest.assertTrue(queueIntAdd.add(i));
        }
        arktest.assertEQ(queueIntAdd.getEnd(), elementsNum - 1);
    });

    arrayConcurrentQueueTestsuite.addTest('AddTestStr002', () => {
        let queueStrAdd: containers.stackless.ArrayConcurrentQueue<String> =
            new containers.stackless.ArrayConcurrentQueue<String>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            arktest.assertTrue(queueStrAdd.add('test'));
        }
        arktest.assertTrue(queueStrAdd.add('teststring'));
        arktest.assertEQ(queueStrAdd.getEnd(), 'teststring');
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('AddTestInt006', async () => {
        let job1 = launch(queueIntAddData);
        let job2 = launch(queueIntAddData);
        let job3 = launch(queueIntAddData);
        let job4 = launch(queueIntAddData);
        job1.Await();
        job2.Await();
        job3.Await();
        job4.Await();
        arktest.assertFalse(queueIntPopQueue.isEmpty());
        for (let i: int = 0; i < 4; ++i) {
            let result = await queueIntPopQueue.pop();
            arktest.assertEQ(result, 1);
        }
        arktest.assertTrue(queueIntPopQueue.isEmpty());
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('AddTestInt007', async () => {
        for (let i: int = 0; i < elementsNum; ++i) {
            arktest.assertTrue(queueInt.add(i));
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getFirst(), 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('AddTestInt008', async () => {
        while (!queueInt.isEmpty()) {
            arktest.assertNE(await queueInt.pop(), undefined);
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            arktest.assertTrue(queueInt.add(i));
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getEnd(), elementsNum - 1);
    });

    arrayConcurrentQueueTestsuite.addTest('AddElementInFullQueueInt009', () => {
        let queueSize = 10;
        let queue: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(queueSize);
        for (let i: int = 0; i < queueSize; ++i) {
            arktest.assertTrue(queue.add(i));
        }
        let queueItem = 1;
        arktest.assertFalse(queue.add(queueItem));
        arktest.assertEQ(queue.size, queueSize);
        arktest.assertEQ(queue.getEnd(), queueSize - 1);
        arktest.assertEQ(queue.getFirst(), 0);
        arktest.assertEQ(queue.size, queueSize);
    });

    return arrayConcurrentQueueTestsuite.run();
}

function queueIntAddData() {
    arktest.assertTrue(queueIntPopQueue.add(1));
}
