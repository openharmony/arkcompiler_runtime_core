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

let counter = 0;

type L = StdDebug.Logger

function stackEater(): void {
    stackEater();
}

function coroStackOverflow(): Object | null {
    stackEater();
    return null;
}

function coroError(): Object {
    throw new Error();
}

async function launchStackOverflow(): Promise<boolean> {
    let overflow: boolean = false;

    try {
        await launch(coroStackOverflow);
        L.log('No errors thrown by coroStackOverflow() but should be!')
        overflow = false;
    } catch (e) {
        if (!(e instanceof StackOverflowError)) {
            L.log('Expected StackOverflowError but another error has been thrown!');
            overflow = false;
        } else {
            overflow = true;
        }
    }

    return overflow;
}

async function launchError(): Promise<boolean> {
    let hasError: boolean = false;

    try {
        await launch(coroError);
        L.log('No errors thrown by coroError() but should be!')
        hasError = false;
    } catch (e) {
        if (!(e instanceof Error)) {
            L.log('Expected Error but another error has been thrown!');
            hasError = false;
        } else {
            hasError = true;
        }
    }

    return hasError;
}

async function testLaunchStackOverflow(): Promise<void> {
    let res: boolean = await launchStackOverflow();
    arktest.assertEQ(res, true);
}

async function testLaunchError(): Promise<void> {
    let res: boolean = await launchError();
    arktest.assertEQ(res, true);
}

async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.launch_error');
    testSuite.addAsyncTest('testLaunchStackOverflow', testLaunchStackOverflow);
    testSuite.addAsyncTest('testLaunchError', testLaunchError);
    let res = await testSuite.runAsync();
    return res;
}
