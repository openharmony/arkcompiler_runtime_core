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

import {launch} from "std/concurrency";

let flag = false;

type L = StdDebug.Logger

function getString(): String {
    flag = true;
    return 'Paragliding is awesome!!\n';
}

function getInt(): int {
    flag = true;
    let ret: int = 0;
    return ret;
}

function getNumber(): number {
    flag = true;
    let ret: number = 0;
    return ret;
}

function getBoolean(): boolean {
    flag = true;
    let ret: boolean = false;
    return ret;
}

function getObject(): Object {
    flag = true;
    let func = (a : String) => { return a; }
    return func;
}

// template function for return types check
async function checkReturnType<T>(fn: () => T): Promise<void> {
    // reset flag
    flag = false;

    let p = launch(fn);
    let result = await p;

    if (!flag) {
        arktest.assertTrue(false, 'Flag is not set!');
        return;
    } else {
        L.log('First await result: ' + result);
        let result2 = await p;
        L.log('Second await result: ' + result2);
        if (!(result === result2)) {
            arktest.assertTrue(false, 'await results do not match!');
            return;
        }
    }
}

async function testAwaitForString(): Promise<void> {
    await checkReturnType(getString);
}

async function testAwaitForInt(): Promise<void> {
    await checkReturnType(getInt);
}

async function testAwaitForNumber(): Promise<void> {
    await checkReturnType(getNumber);
}

async function testAwaitForBoolean(): Promise<void> {
    await checkReturnType(getBoolean);
}

async function testAwaitForObject(): Promise<void> {
    await checkReturnType(getObject);
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.await');
    testSuite.addAsyncTest('testAwaitForString', testAwaitForString);
    testSuite.addAsyncTest('testAwaitForInt', testAwaitForInt);
    testSuite.addAsyncTest('testAwaitForNumber', testAwaitForNumber);
    testSuite.addAsyncTest('testAwaitForBoolean', testAwaitForBoolean);
    testSuite.addAsyncTest('testAwaitForObject', testAwaitForObject);
    let res = await testSuite.runAsync();
    return res;
}
