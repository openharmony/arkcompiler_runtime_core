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

import { launch } from "std/concurrency"

function main(): int {
    let ArrayBlockingQueueTestsuite = new arktest.ArkTestsuite("ArrayBlockingQueueGetSizeTest");

    ArrayBlockingQueueTestsuite.addAsyncTest("GetSizeTestInt003", async () =>
    {
        const elementsNum: int = 100;
        let queueInt = new containers.stackless.ArrayBlockingQueue<int>(elementsNum * 2);
        let promises = new Array<Promise<void>>(elementsNum);
        for (let i: int = 0; i < elementsNum; ++i) {
            promises[i] = queueInt.push(i);
            arktest.assertTrue(queueInt.add(i));
        }
        for (let i: int = 0; i < elementsNum; i++) {
            await promises[i];
        }
        arktest.assertEQ(queueInt.size, elementsNum * 2)
    });

    return ArrayBlockingQueueTestsuite.run();
}
