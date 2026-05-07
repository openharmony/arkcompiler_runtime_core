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

async function AsyncFailing(): Promise<string> {
    throw Error('AsyncFailing');
}

function Failing(): string {
    throw Error('Failing');
}

function SingleRejectedJob(): null {
    throw Error('This is a single rejected job');
}

async function testAllhandledRejection(): Promise<void> {
    StdProcess.on('unhandledPromiseRejection', (reason: Error, obj: Object): void => {
        arktest.assertTrue(obj instanceof Promise, 'The object should be of type Promise');
    });

    let g = launch(Failing);

    try {
        await launch (Failing);
    } catch(e) {
    }

    AsyncFailing().catch((e: Any) => {});

    let p = Promise.reject(new Error('promise1'));
    let q = p.then(() => {
        L.log('Test failed: unreachable code');
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    });
    q.catch((e: Any) => {});

    let f = async (): Promise<void> => {
        try {
            await Promise.reject(new Error('promise2'));
        } catch (e) {
        }
    };

    f();

    try {
        await g;
    } catch (e) {
    }

    launch(SingleRejectedJob);
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.allhandled_rejection');
    testSuite.addAsyncTest('testAllhandledRejection', testAllhandledRejection);
    let res = await testSuite.runAsync();
    return res;
}
