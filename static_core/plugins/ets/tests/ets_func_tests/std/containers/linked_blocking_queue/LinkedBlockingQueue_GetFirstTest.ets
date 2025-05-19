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

function main(): int {
    let LinkedBlockingQueueTestsuite = new arktest.ArkTestsuite("LinkedBlockingQueueGetFirstTest");

    LinkedBlockingQueueTestsuite.addTest("GetFirstTest001", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        let res = queueInt.getFirst();
        arktest.assertEQ(res, 0);
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.size, elementsNum);
    });
    LinkedBlockingQueueTestsuite.addTest("GetFirstTest002", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < elementsNum; i++) {
            arktest.assertEQ(queueInt.add(i), true);
        }
        queueInt.pop();
        let res = queueInt.getFirst();
        arktest.assertEQ(res, 1);
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.size, elementsNum - 1);
    });
    LinkedBlockingQueueTestsuite.addTest("GetFirstAndSizeAndPopAndPollTest009", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.size, 20 - i);
            queueInt.pop();
        }
        arktest.assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            arktest.assertEQ(queueInt.size, 10 - i);
            queueInt.poll();
        }
        arktest.assertEQ(queueInt.size, 0);
    });
    LinkedBlockingQueueTestsuite.addTest("GetFirstAndIsEmptyAndPopAndPollTest010", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>();
        for (let i = 0; i < 10; i++) {
            queueInt.add(i);
        }
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 10; i < 20; i++) {
            queueInt.push(i);
        }
        arktest.assertEQ(queueInt.isEmpty(), false);
        arktest.assertEQ(queueInt.getFirst(), 0);
        for (let i = 0; i < 10; i++) {
            queueInt.pop();
        }
        arktest.assertEQ(queueInt.getFirst(), 10);
        for (let i = 0; i < 10; i++) {
            queueInt.poll();
        }
        arktest.assertEQ(queueInt.isEmpty(), true);
    });
    LinkedBlockingQueueTestsuite.addTest("GetFirstStressTest005", () =>
    {
        let queueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);
        for (let i = 0; i < stressNum; i++) {
            queueInt.add(i);
        }
        for (let i = 0; i < stressNum; i++) {
            arktest.assertEQ(queueInt.getFirst(), i);
            queueInt.pop();
        }
        arktest.assertEQ(queueInt.size, 0);
    });

    return LinkedBlockingQueueTestsuite.run();
}