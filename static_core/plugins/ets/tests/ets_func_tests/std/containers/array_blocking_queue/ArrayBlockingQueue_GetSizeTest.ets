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

import { launch } from "std/concurrency"

const elementsNum: int = 100;
let sizeNum: int = 0;

let queueInt: ArrayBlockingQueue<int> = new ArrayBlockingQueue<int>(200);

function main(): int {
    let ArrayBlockingQueueTestsuite = new ArkTestsuite("ArrayBlockingQueueGetSizeTest");

    ArrayBlockingQueueTestsuite.addTest("GetSizeTestInt003", () =>
    {
        while (!queueInt.isEmpty()) {
            queueInt.pop();
        }
        for (let i: int = 0; i < elementsNum; ++i) {
            queueInt.push(i);
        }
        let p1 = launch<void, () => void>(queueIntSizedata);
        let p2 = launch<void, () => void>(queueIntSizedata);
        let p3 = launch<void, () => void>(queueIntSizedata);
        let p4 = launch<void, () => void>(queueIntSizedata);
        p1.Await(); 
        p2.Await(); 
        p3.Await(); 
        p4.Await(); 
        assertEQ(queueInt.size, elementsNum);
    });

    return ArrayBlockingQueueTestsuite.run();
}

function queueIntSizedata() {
    sizeNum = queueInt.size;
}