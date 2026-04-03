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

let currentValue: int = 0;
const numbers: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

function main(): int {
    let arrayConcurrentQueueTestsuite = new arktest.ArkTestsuite('ArrayConcurrentQueueConstructorTest');

    arrayConcurrentQueueTestsuite.addTest('ConstructorTestInt001', () => {
        let queueInt: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200);
        arktest.assertEQ(queueInt.isEmpty(), true);
        arktest.assertEQ(queueInt.size, 0);
    });

    arrayConcurrentQueueTestsuite.addAsyncTest('ConstructorTestInt002', async () => {
        let array = Array.from<int>(numbers);
        let concurrentSet = new containers.ConcurrentSet<int>(array);
        arktest.assertEQ(concurrentSet.size, 10);
        currentValue = 0;
        concurrentSet.forEach((value: int) => {
            arktest.assertEQ(value, currentValue++);
        });
        let iterator = concurrentSet.keys();
        let queueInt: containers.stackless.ArrayConcurrentQueue<int> =
            new containers.stackless.ArrayConcurrentQueue<int>(200, iterator);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(await queueInt.pop(), i);
        }
    });

    return arrayConcurrentQueueTestsuite.run();
}