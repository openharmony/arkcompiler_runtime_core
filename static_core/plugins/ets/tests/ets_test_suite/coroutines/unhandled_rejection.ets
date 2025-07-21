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

import {AtomicFlag} from "std/debug/concurrency"

class Event {
    constructor() {
        this.promise = new Promise<Number>((resolve: (val: Number) => void) => {
            this.resolveFn = resolve;
        })
    }

    public Wait() {
        await this.promise;
    }

    public Fire() {
        this.resolveFn!(0);
    }

    private promise: Promise<Number>;
    private resolveFn: ((val: Number) => void) | null = null;
}


const kTotal = 5;
let current = 0;
let event = new Event();

async function AsyncFailing(): Promise<string> {
    throw Error("AsyncFailing");
}

function Failing(): string {
    throw Error("Failing");
}

function WaitAll(): null {
    event.Wait();
    return null;
}

function Handler(obj: Object): void {
    if (current == 0) {
        launch<null, () => null>(WaitAll);
    }
    current++;
    if (current == kTotal) {
        event.Fire();
    } else {
        assertTrue(current < kTotal, "More rejections that expected");
    }
}

function main(): void {
    StdProcess.on("unhandledJobRejection", (obj: Object): void => {
        assertTrue(false, "Job handler didn't switch");
    });
    StdProcess.on("unhandledPromiseRejection", (obj: Object): void => {
        assertTrue(false, "Promise handler didn't switch");
    });

    StdProcess.on("unhandledJobRejection", Handler);
    StdProcess.on("unhandledPromiseRejection", Handler);

    let g = launch<string, () => string>(Failing);
    launch<Promise<string>, () => Promise<string>>(AsyncFailing);
    AsyncFailing();
    let p = Promise.reject(new Error("promise1"));
    let q = p.then(() => {
        console.println("Test failed: Unreachable code");
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    })
    Promise.reject(new Error("promise2"));
}
