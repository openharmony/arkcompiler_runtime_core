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
const stressNum: int = 100;

let sMTQueueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(stressNum);

function main(): int {
    let ArrayBlockingQueueTestsuite = new ArkTestsuite("ArrayBlockingQueuePollTest");

    ArrayBlockingQueueTestsuite.addTest("PollTest001", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);
        for (let i = 0; i < elementsNum; i++) {
            assertEQ(queueInt.add(i), true);
        }
        assertEQ(queueInt.size, elementsNum);
        for (let i = 0; i < 5; i++) {
            assertEQ(queueInt.poll(), i);
        }
        assertEQ(queueInt.size, 5);
        for (let i = 5; i < 10; i++) {
            assertEQ(queueInt.poll(), i);
        }
        assertEQ(queueInt.size, 0);
        assertEQ(queueInt.poll(), null);
    });
    ArrayBlockingQueueTestsuite.addTest("PollStressTest004", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < stressNum; i++) {
            assertEQ(queueInt.poll(), i);
        }
        assertEQ(queueInt.size, 0);
    });
    ArrayBlockingQueueTestsuite.addTest("ThreadsPollStressTest003", () =>
    {
        while (!sMTQueueInt.isEmpty()) {
            sMTQueueInt.pop();
        }
        for (let i = 0; i < stressNum; i++) {
            sMTQueueInt.add(i);
        }
        let promises = new Array<Promise<void>>(100);
        let p = 0;
        for (let i = 0; i < stressNum; i++) {
            promises[p++] = launch sQueuePollData();
        }
        for (let i: int = 0; i < 100; i++) {
            await promises[i];
        }
        assertEQ(sMTQueueInt.size, 0);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function sQueuePollData() {
    sMTQueueInt.poll();
}