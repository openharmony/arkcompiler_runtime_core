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
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueueCapacityTest");

    LinkedBlockingQueueTestsuite.addTest("CapacityTest001", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(elementsNum);
        for (let i = 0; i < 5; i++) {
            assertEQ(queueInt.add(i), true);
        }
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.size, 5);
        assertEQ(queueInt.capacity, elementsNum);
        assertEQ(queueInt.add(testNum), true);
        assertEQ(queueInt.size, 6);
        assertEQ(queueInt.capacity, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("CapacityTest007", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.capacity, stressNum);
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        assertEQ(queueInt.capacity, stressNum);
    });
    LinkedBlockingQueueTestsuite.addTest("GetCapacityStressTest008", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
            assertEQ(queueInt.capacity, stressNum);
        }
        assertEQ(queueInt.size, stressNum);
    });

    return LinkedBlockingQueueTestsuite.run();
}
