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

function main(): int {
    let ArrayBlockingQueueTestsuite = new ArkTestsuite("ArrayBlockingQueueRemainingCapacityTest");

    ArrayBlockingQueueTestsuite.addTest("RemainingCapacityTest001", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(elementsNum);
        for (let i = 0; i < 5; i++) {
            assertEQ(queueInt.add(i), true);
        }
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.size, 5);
        assertEQ(queueInt.remainingCapacity(), 5);
        assertEQ(queueInt.add(testNum), true);
        assertEQ(queueInt.size, 6);
        assertEQ(queueInt.remainingCapacity(), 4);
    });
    ArrayBlockingQueueTestsuite.addTest("RemainingCapacityTest006", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(stressNum);
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.remainingCapacity(), stressNum - i);
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        assertEQ(queueInt.remainingCapacity(), stressNum - 20);
    });
    ArrayBlockingQueueTestsuite.addTest("RemainingCapacityStressTest009", () =>
    {
        let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(stressNum);
        let j: int = stressNum;
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
            j--;
            assertEQ(queueInt.remainingCapacity(), j);
        }
        assertEQ(queueInt.size, stressNum);
    });

    return ArrayBlockingQueueTestsuite.run();
}
