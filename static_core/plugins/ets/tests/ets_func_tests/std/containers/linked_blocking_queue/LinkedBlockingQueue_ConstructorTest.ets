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

let flag: int = 0;
const elementsNum: int = 10;
const stressNum: int = 100;
const arr: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

function main(): int {
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueueConstructorTest");

    LinkedBlockingQueueTestsuite.addTest("ConstructorTest001", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        assertEQ(queueInt.isEmpty(), true);
        assertEQ(queueInt.size, 0);
    });
    LinkedBlockingQueueTestsuite.addTest("ConstructorTest002", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(elementsNum);
        assertEQ(queueInt.isEmpty(), true);
        assertEQ(queueInt.size, 0);
        assertEQ(queueInt.capacity, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("ConstructorTest003", () =>
    {
        let array = Array.from<number>(arr);
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>(array);
        assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: number) => {
            assertEQ(val, flag++);
        });
        let iter = ConcurrentSetTest.keys();
        let queueInt: LinkedBlockingQueue<number> = new LinkedBlockingQueue<number>(iter);
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.pop(), i);
        }
        assertEQ(queueInt.capacity, Int.MAX_VALUE);
    });
    LinkedBlockingQueueTestsuite.addTest("ConstructorTest004", () =>
    {
        let array = Array.from<number>(arr);
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>(array);
        assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: number) => {
            assertEQ(val, flag++);
        });
        let iter = ConcurrentSetTest.keys();
        let queueInt: LinkedBlockingQueue<number> = new LinkedBlockingQueue<number>(10, iter);
        for (let i = 0; i < 10; i++) {
            assertEQ(queueInt.pop(), i);
        }
        assertEQ(queueInt.capacity, 10);
    });

    return LinkedBlockingQueueTestsuite.run();
}