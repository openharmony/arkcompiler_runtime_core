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

let sMTQueueInt: LinkedBlockingQueue<int> = new LinkedBlockingQueue<int>(stressNum);

function main(): int {
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueueAddTest");

    LinkedBlockingQueueTestsuite.addTest("ThreadsAddStressTest001", () =>
    {
        while (!sMTQueueInt.isEmpty()) {
            sMTQueueInt.pop();
        }
        let promises = new Array<Promise<void>>(100);
        let p = 0;
        for (let i = 0; i < 100; i++) {
            promises[p++] = launch sQueueAddData(i);
        }
        for (let i: int = 0; i < 100; i++) {
            await promises[i];
        }
        assertEQ(sMTQueueInt.size, 100);
    });

    return LinkedBlockingQueueTestsuite.run();
}

function sQueueAddData(idx: int) {
    sMTQueueInt.add(idx);
}
