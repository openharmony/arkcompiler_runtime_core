/*
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
import { Job } from "std/core"

let ConcurrentSetTestOne: ConcurrentSet<number> = new ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new ArkTestsuite("ConcurrentSetPressureTest");

    ConcurrentSetTestsuite.addTest("setPressureTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        let jobs = new Array<Job<void>>(100);

        for (let i: int = 0; i < 100; i++) {
            jobs[i] = launch<void, (i: int) => void>(addMore, i);
        }
        for (let i: int = 0; i < 100; i++) {
            jobs[i].Await();
        }
        assertEQ(ConcurrentSetTestOne.size, 100);
    });

    return ConcurrentSetTestsuite.run();
}

function addMore(i: int) {
    ConcurrentSetTestOne.add(i);
}
