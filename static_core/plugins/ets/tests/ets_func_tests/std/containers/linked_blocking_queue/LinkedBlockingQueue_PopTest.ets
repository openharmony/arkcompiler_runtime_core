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

let sMTQueueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);

function main(): int {
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueuePopTest");

    LinkedBlockingQueueTestsuite.addTest("PopTest001", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < elementsNum; i++) {
            assertEQ(queueInt.add(i), true);
        }
        let res = queueInt.getFirst();
        assertEQ(res, 0);
        assertEQ(queueInt.isEmpty(), false);
        assertEQ(queueInt.size, elementsNum);
        for (let i = 0; i < elementsNum; i++) {
            queueInt.pop();
        }
        assertEQ(queueInt.isEmpty(), true);
        assertEQ(queueInt.size, 0);
    });
    LinkedBlockingQueueTestsuite.addTest("PopStressTest003", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < stressNum; i++) {
            queueInt.pop();
        }
        assertEQ(queueInt.size, 0);
    });

    return LinkedBlockingQueueTestsuite.run();
}
