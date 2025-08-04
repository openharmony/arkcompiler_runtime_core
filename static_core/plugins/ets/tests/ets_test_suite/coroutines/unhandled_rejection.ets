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

type L = StdDebug.Logger

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
const asynclock = new AsyncLock();
let current = 0;
let event = new Event();

async function AsyncFailing(): Promise<string> {
    throw Error('AsyncFailing');
}

function Failing(): string {
    throw Error('Failing');
}

function Handler(reason: Error, obj: Object): void {
    await asynclock.lockAsync(() => {
        current++;
        if (current == kTotal) {
            event.Fire();
        } else {
            arktest.assertTrue(current < kTotal, 'More rejections that expected');
        }
    },  AsyncLockMode.EXCLUSIVE);
}

function testUnhandledRejection(): void {
    StdProcess.on('unhandledJobRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Job, 'The object should be of type Job');
        arktest.assertEQ(reason.message, 'Failing');
    });
    StdProcess.on('unhandledPromiseRejection',  (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
        Handler(reason, obj);
    });

    let g = launch<string, () => string>(Failing);
    launch<Promise<string>, () => Promise<string>>(AsyncFailing);
    AsyncFailing();
    let p = Promise.reject(new Error('promise1'));
    let q = p.then(() => {
        L.log('Test failed: Unreachable code');
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    })
    Promise.reject(new Error('promise2'));
    Promise.reject();
    event.Wait();
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.unhandled_rejection');
    testSuite.addTest('testUnhandledRejection', testUnhandledRejection);
    let res = testSuite.run();
    return res;
}
