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

let flag: int = 0;
const arr: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

function main() {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueConstructorTest");

    ArrayBlockingQueueTestsuite.addTest("ConstructorTestInt001", () =>
    {
        let queueIntA: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200);
        arktest.assertEQ(queueIntA.isEmpty(), true);
        arktest.assertEQ(queueIntA.size, 0);
    });
    ArrayBlockingQueueTestsuite.addAsyncTest("ConstructorTestInt002", async () =>
    {
        let array = Array.from<int>(arr);
        let ConcurrentSetTest: containers.ConcurrentSet<int> = new containers.ConcurrentSet<int>(array);
        arktest.assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: int) => {
            arktest.assertEQ(val, flag++);
        });
        let iter = ConcurrentSetTest.keys();
        let queueIntA: containers.stackless.ArrayBlockingQueue<int> = new containers.stackless.ArrayBlockingQueue<int>(200, iter);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(await queueIntA.pop(), i);
        }
    });

    ArrayBlockingQueueTestsuite.run();
}