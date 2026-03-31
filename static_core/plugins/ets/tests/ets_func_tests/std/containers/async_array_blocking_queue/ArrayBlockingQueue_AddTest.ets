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

let queueInt: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);
let queueIntPopN: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueAddTest");

    ArrayBlockingQueueTestsuite.addTest("AddTestInt001", () =>
    {
        let queueIntAdd: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            arktest.assertTrue(queueIntAdd.add(i));
        }
        arktest.assertEQ(queueIntAdd.getEnd(), elementsNum - 1);
    });
    ArrayBlockingQueueTestsuite.addTest("AddTestStr002", () =>
    {
        let queueStrAdd: containers.stackless.ArrayBlockingQueue<String> = new containers.stackless.ArrayBlockingQueue<String>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            arktest.assertTrue(queueStrAdd.add("test"));
        }
        arktest.assertTrue(queueStrAdd.add("teststring"));
        arktest.assertEQ(queueStrAdd.getEnd(), "teststring");
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("AddTestInt006", async () =>
    {
        let p1 = launch(queueIntAdddata);
        let p2 = launch(queueIntAdddata);
        let p3 = launch(queueIntAdddata);
        let p4 = launch(queueIntAdddata);
        p1.Await();
        p2.Await();
        p3.Await();
        p4.Await();
        arktest.assertFalse(queueIntPopN.isEmpty());
        for (let i: int = 0; i < 4; ++i) {
            let res = await queueIntPopN.pop();
            arktest.assertEQ(res, 1);
        }
        arktest.assertTrue(queueIntPopN.isEmpty());
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("AddTestInt007", async () =>
    {
        for (let i: int = 0; i < elementsNum; ++i) {
            arktest.assertTrue(queueInt.add(i));
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getFirst(), 0);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("AddTestInt008", async () =>
    {
        while (!queueInt.isEmpty()) {
            arktest.assertNE(await queueInt.pop(), undefined);
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            arktest.assertTrue(queueInt.add(i));
        }
        arktest.assertEQ(queueInt.size, elementsNum);
        arktest.assertEQ(queueInt.getEnd(), elementsNum - 1);
    });
    ArrayBlockingQueueTestsuite.addTest("AddElementInFullQueueInt009", () =>
    {
        let size = 10;
        let queue: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(size);
        for (let i: int = 0; i < size; ++i) {
            arktest.assertTrue(queue.add(i));
        }
        let el = 1;
        arktest.assertFalse(queue.add(el));
        arktest.assertEQ(queue.size, size);
        arktest.assertEQ(queue.getEnd(), size - 1)
        arktest.assertEQ(queue.getFirst(), 0)
        arktest.assertEQ(queue.size, size);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntAdddata() {
    arktest.assertTrue(queueIntPopN.add(1));
}
