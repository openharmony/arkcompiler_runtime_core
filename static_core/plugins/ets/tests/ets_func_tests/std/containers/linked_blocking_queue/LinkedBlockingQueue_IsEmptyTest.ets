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

const stressNum: int = 100;

function main(): int {
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueueIsEmptyTest");

    LinkedBlockingQueueTestsuite.addTest("IsEmptyStressTest006", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < stressNum; i++) {
            assertEQ(queueInt.isEmpty(), false);
        }
        for (let i = 0; i < stressNum; i++) {
            queueInt.pop();
        }
        for (let i = 0; i < stressNum; i++) {
            assertEQ(queueInt.isEmpty(), true);
        }
    });

    return LinkedBlockingQueueTestsuite.run();
}
