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

const elementsNum: int = 100;
let b1: int | null = 0;
let f1: int | null = 0;

let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
let queueIntPopN: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new ArkTestsuite("ArrayBlockingQueueAddTest");

    ArrayBlockingQueueTestsuite.addTest("AddTestInt001", () =>
    {
        let queueIntAdd: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueIntAdd.add(i);
        }
        assertEQ(queueIntAdd.getEnd(), elementsNum - 1);
    });
    ArrayBlockingQueueTestsuite.addTest("AddTestStr002", () =>
    {
        let queueStrAdd: ArrayBlockingQueue<String> = new ArrayBlockingQueue<String>(200);
        for (let i: int = 0; i < elementsNum; ++i) {
            queueStrAdd.add("test");
        }
        queueStrAdd.add("teststring");
        assertEQ(queueStrAdd.getEnd(), "teststring");
    });
    ArrayBlockingQueueTestsuite.addTest("AddTestInt006", () =>
    {
        while (!queueIntPopN.isEmpty()) {
            queueIntPopN.pop();
        }
        let p1 = launch<void, () => void>(queueIntAdddata);
        let p2 = launch<void, () => void>(queueIntAdddata);
        let p3 = launch<void, () => void>(queueIntAdddata);
        let p4 = launch<void, () => void>(queueIntAdddata);
        p1.Await();
        p2.Await();
        p3.Await();
        p4.Await();
        assertFalse(queueIntPopN.isEmpty());
    });
    ArrayBlockingQueueTestsuite.addTest("AddTestInt007", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.add(i);
        }
        let p1 = launch<void, () => void>(queueIntFrontF1data);
        p1.Await();
        assertEQ(queueInt.size, elementsNum);
    });
    ArrayBlockingQueueTestsuite.addTest("AddTestInt008", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.add(i);
        }
        let p1 = launch<void, () => void>(queueIntBackB1data);
        p1.Await();
        assertFalse(queueInt.isEmpty());
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntAdddata() {
    queueIntPopN.add(1);
}

function queueIntBackB1data() {
    b1 = queueInt.getEnd();
}

function queueIntFrontF1data() {
    f1 = queueInt.getFirst();
}
