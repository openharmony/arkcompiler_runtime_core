/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

type L = StdDebug.Logger

function returnValue(i: int): String {
    return 'value ' + i;
}

async function launchCoroutines(n_cor: int): Promise<int> {
    let p: (Promise<String> | undefined)[] = new (Promise<String> | undefined)[n_cor](undefined);
    for (let i = 0; i < n_cor; ++i) {
        p[i] = launch<String>() {
            return returnValue(i)
        };
    }
    for (let i = 0; i < n_cor; ++i) {
        let val = await (p[i]!);
        if(val != returnValue(i)) {
            L.log('p[' + i + '] = ' + val + ' instead of ' + returnValue(i));
            return 1;
        }
    }
    return 0;
}

function throwError(): Object {
    throw new Error();
}

async function launchCoroutinesWithError(n_cor: int): Promise<int> {
    let p: (Promise<Object> | undefined)[] = new (Promise<Object> | undefined)[n_cor](undefined);
    for (let i = 0; i < n_cor; ++i) {
        p[i] = launch(throwError);
    }
    for (let i = 0; i < n_cor; ++i) {
        try {
            await (p[i]!);
        } catch (e) {
            if (!(e instanceof Error)) {
              L.log('Expected Error but another error has been thrown!: ' + e);
              return 1;
            }
            continue;
        }
        L.log('Was no error throwing');
        return 1;
    }
    return 0;
}

async function testMultipleLaunch(): Promise<void> {
    if(await launchCoroutines(5) != 0) {
        arktest.assertTrue(false, 'First launch of 5 coroutines failed!!');
        return;
    }

    if(await launchCoroutines(5) != 0) {
        arktest.assertTrue(false, 'Second launch of 5 coroutines failed!!');
        return;
    }

    if(await launchCoroutines(10) != 0) {
        arktest.assertTrue(false, 'First launch of 10 coroutines failed!!');
        return;
    }

    if(await launchCoroutines(10) != 0) {
        arktest.assertTrue(false, 'Second launch of 10 coroutines failed!!');
        return;
    }

    if(await launchCoroutinesWithError(5) != 0) {
        arktest.assertTrue(false, 'First launch of 5 throw-error coroutines failed!!');
        return;
    }

    if(await launchCoroutines(10) != 0) {
        arktest.assertTrue(false, 'First launch of 10 coroutines after 5 throw-error ones failed!!');
        return;
    }
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.multiple_launch');
    testSuite.addAsyncTest('testMultipleLaunch', testMultipleLaunch);
    let res = await testSuite.runAsync();
    return res;
}
