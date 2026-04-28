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

type L = StdDebug.Logger

const kTotal = 7;
const asynclock = new AsyncLock();
let current = 0;

async function AsyncFailing(): Promise<string> {
    throw Error('AsyncFailing');
}

function Failing(): string {
    throw Error('Failing');
}

// Called before or on the program completion
function PromiseHandler(reason: Error, obj: Object): void {
    // should be no-throw
    asynclock.lockAsync((): void => {
        current++;
        if (current > kTotal) {
            console.error('More rejections than expected');
            let procManager = new StdProcess.ProcessManager();
            procManager.exit(1);
        }
    },  AsyncLockMode.EXCLUSIVE);
}

function testUnhandledRejection(): void {
    StdProcess.on('unhandledPromiseRejection',  (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
        PromiseHandler(reason, obj);
    });

    let g = launch(Failing);
    launch(AsyncFailing);
    AsyncFailing();
    let p = Promise.reject(new Error('promise1'));
    let q = p.then(() => {
        L.log('Test failed: Unreachable code');
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    })
    Promise.reject(new Error('promise2'));
    Promise.reject();
}

async function main(): Promise<int> {
    StdProcess.off('unhandledJobRejection' as 'unhandledPromiseRejection');
    let testSuite = new arktest.ArkTestsuite('coroutines.unhandled_rejection');
    testSuite.addTest('testUnhandledRejection', testUnhandledRejection);
    let res = await testSuite.runAsync();
    return res;
}
