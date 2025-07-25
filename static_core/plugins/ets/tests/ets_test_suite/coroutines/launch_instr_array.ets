/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

// This test covers case of storing Promises returned from launch instruction
// into array for different modes: INT, AOT, JIT

class TestCoroutine {
    public static threadsCount: int = 2;
    public static jArray: (Job<Object> | null)[] = [null, null];

    public jCor(): Job<Object> {
        const factory = () => {return new Object();}
        return launch<Object, () => Object>(factory);
    }

    public run(): void {
        for(let i = 0; i < TestCoroutine.threadsCount; i++) {
            TestCoroutine.jArray[i] = this.jCor();
        }
        Coroutine.Schedule();
    }
}

function main(): void {
    let a = new TestCoroutine();
    a.run();
}