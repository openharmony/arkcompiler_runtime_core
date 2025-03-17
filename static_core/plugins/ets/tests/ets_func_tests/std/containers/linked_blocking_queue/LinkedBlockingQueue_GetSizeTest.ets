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
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueueSizeTest");

    LinkedBlockingQueueTestsuite.addTest("SizeTest001", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < elementsNum; i++) {
            assertEQ(queueInt.add(i), true);
        }
        assertEQ(queueInt.size, elementsNum);
        for (let i = 0; i < 5; i++) {
            queueInt.pop();
        }
        assertEQ(queueInt.size, 5);
        assertEQ(queueInt.add(testNum), true);
        assertEQ(queueInt.size, 6);
    });
    LinkedBlockingQueueTestsuite.addTest("SizeAndPopAndPollTest008", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.size, 20 - i);
            queueInt.pop();
        }
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.size, 10 - i);
            queueInt.poll();
        }
        assertEQ(queueInt.size, 0);
    });
    LinkedBlockingQueueTestsuite.addTest("GetSizeStressTest007", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            assertEQ(queueInt.size, i);
            queueInt.add(i);
        }
        assertEQ(queueInt.size, stressNum);
    });

    return LinkedBlockingQueueTestsuite.run();
}
