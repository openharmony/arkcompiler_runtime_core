/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import {AtomicFlag} from "std/debug/concurrency"

import { launch } from "std/concurrency"
import { Job } from "std/core"

class Event {
    public Wait() {
        while (this.flag.get() != true) {
            Coroutine.Schedule();
        }
    }

    public Fire() {
        this.flag.set(true);
    }

    private flag = new AtomicFlag(false);
};

let event = new Event();

function foo() : int {
    event.Wait();
    return 42;
}

function bar(p: Job<int>) : int {
    return p.Await();
}

function main() {
    let p1 = launch<int, () => int>(foo);
    for (let i = 0; i < 2; ++i) {
        let p2 = launch<int, (p: Job<int>) => int>(bar, p1);
    }
    // NOTE(panferovi): for more determinism we need to make sure
    // that bar coroutines are waiting for its awakening
    event.Fire();
    let result = p1.Await();
    assertEQ(result, 42);
}
