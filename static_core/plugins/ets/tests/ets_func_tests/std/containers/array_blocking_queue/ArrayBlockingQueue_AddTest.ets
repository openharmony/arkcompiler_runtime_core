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
let queueIntPopN: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueAddTest");

    ArrayBlockingQueueTestsuite.addTest("AddTestInt001", () =>
    {
        let queueIntAdd: containers.ArrayBlockingQueue<int> = new containers.ArrayBlockingQueue<int>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntAdd.add(i);
        }
        arktest.assertEQ(queueIntAdd.getEnd(), elementsNum - 1);
    });
    ArrayBlockingQueueTestsuite.addTest("AddTestStr002", () =>
    {
        let queueStrAdd: containers.ArrayBlockingQueue<String> = new containers.ArrayBlockingQueue<String>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueStrAdd.add("test");
        }
        queueStrAdd.add("teststring");
        arktest.assertEQ(queueStrAdd.getEnd(), "teststring");
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("AddTestInt006", async () =>
    {
        while (!queueIntPopN.isEmpty()) {
            queueIntPopN.pop();
        }
        let p1 = launch(queueIntAdddata);
        let p2 = launch(queueIntAdddata);
        let p3 = launch(queueIntAdddata);
        let p4 = launch(queueIntAdddata);
        await p1;
        await p2;
        await p3;
        await p4;
        arktest.assertFalse(queueIntPopN.isEmpty());
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("AddTestInt007", async () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.add(i);
        }
        let p1 = launch(queueIntFrontF1data);
        await p1;
        arktest.assertEQ(queueInt.size, elementsNum);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("AddTestInt008", async () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.add(i);
        }
        let p1 = launch(queueIntBackB1data);
        await p1;
        arktest.assertFalse(queueInt.isEmpty());
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntAdddata() {
    queueIntPopN.add(1);
}

function queueIntBackB1data() {
    queueInt.getEnd();
}

function queueIntFrontF1data() {
    queueInt.getFirst();
}
