/*
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

import { launch } from "std/concurrency"

const INCREMENT = 5;
const EXPECTED_VALUE = 42;

let counter = 0;

function clearCounter() {
    counter = 0;
}

function foo(): number {
    return EXPECTED_VALUE;
}

async function fooAsync(): Promise<number> {
    return foo();
}

function add(num: int) {
    counter += num;
}

async function addAsync(num: number) {
    await Promise.resolve('a');
    counter += num;
}

function throwLaunchError() {
    throw new Error();
}

async function throwAsyncError(): Promise<void> {
    throw new Error();
}

async function testFooLaunch(): Promise<number> {
    let p: Promise<number> = launch(foo);
    let result = await p;

    return result;
}

function testAddSchedule(): void {
    clearCounter();
    let p: Promise<void> = launch() {
        add(INCREMENT)
    }

    Coroutine.Schedule();
}

async function recursiveFoo(): Promise<number> {
    let p: Promise<Promise<number>> = launch(testFooLaunch);
    let result = await p;

    return result;
}

async function testRecursiveLaunch(): Promise<number> {
    let p: Promise<Promise<number>> = launch(recursiveFoo);
    let result = await p;

    return result;
}

async function testLaunchThrow(): Promise<boolean> {
    try {
        let p: Promise<void> = launch(throwLaunchError);
        await p;
    } catch (e) {
        return e instanceof Error;
    }

    return false;
}

function testFooAsync(): number {
    return fooAsync().awaitSync();
}

function testAddAsyncSchedule(): void {
    clearCounter();

    addAsync(INCREMENT);
    Coroutine.Schedule();
}

async function recursiveAsyncFoo(): Promise<number> {
    return fooAsync();
}

function testRecursiveAsync(): number {
    return recursiveAsyncFoo().awaitSync();
}

async function testAsyncThrow(): Promise<boolean> {
    try {
        let p: Promise<void> = throwAsyncError();
        await p;
    } catch (e) {
        return e instanceof Error;
    }

    return false;
}

async function testACoroutineEaworker(): Promise<void> {
    let ew = new EAWorker();
    ew.start();
    let p0 = ew.run<number>(testFooLaunch);
    let p1 = ew.run<void>(testAddSchedule);
    let p2 = ew.run<number>(testRecursiveLaunch);
    let p3 = ew.run<boolean>(testLaunchThrow);

    arktest.assertEQ(EXPECTED_VALUE, await p0);
    await p1;
    arktest.assertEQ(counter, INCREMENT);
    arktest.assertEQ(EXPECTED_VALUE, await p2);
    arktest.assertTrue(await p3);

    await ew.join();
}

async function testAJCoroutineEaworker(): Promise<void> {
    let ew = new EAWorker();
    ew.start();
    let p0 = ew.run<number>(testFooAsync);
    let p1 = ew.run<void>(testAddAsyncSchedule);
    let p2 = ew.run<number>(testRecursiveAsync);
    let p3 = ew.run<boolean>(testLaunchThrow);

    arktest.assertEQ(EXPECTED_VALUE, await p0);
    await p1;
    arktest.assertEQ(counter, INCREMENT);
    arktest.assertEQ(EXPECTED_VALUE, await p2);
    arktest.assertTrue(await p3);

    await ew.join();
}

async function testACAJcoroutine(): Promise<void> {
    let ew = new EAWorker();
    ew.start();
    let p1 = ew.run<number>(testFooLaunch);
    let p2 = ew.run<number>(testFooAsync);

    let res1 = await p1;
    let res2 = await p2;

    await ew.join();

    arktest.assertEQ(res1, res2);
}

async function testACoroutineCallAJCoroutine(): Promise<void> {
    let ew = new EAWorker();
    ew.start();
    let p1 = ew.run<number>(() => {
        return (async (): Promise<number> => {
            let p1: Promise<Promise<number>> = launch(fooAsync)
            let result = await p1;
            return result;
        // NOTE(zhuheng_z, #11066): should be removed after launch will be correctly supported for async functions
        })().awaitSync();
    });
    await ew.join();
    arktest.assertEQ(EXPECTED_VALUE, await p1);
}

async function AsyncCallLaunch(): Promise<Promise<number>> {
    let p: Promise<number> = launch(foo);

    return p;
}

async function testAJCoroutineCallACoroutine(): Promise<void> {
    let ew = new EAWorker();
    ew.start();
    let p1 = ew.run<Promise<number>>(AsyncCallLaunch);
    let result = await p1;
    await ew.join();

    arktest.assertEQ(EXPECTED_VALUE, result);
}

async function testNestedEAWorker(): Promise<void> {
    clearCounter();

    let e1 = new EAWorker();
    e1.start();
    let p1 = e1.run<number>(async () => {
        let e2 = new EAWorker();
        e2.start();
        let p2 = e2.run<number>(():number => {
            return foo();
        });
        e2.quit();

        return await p2;
    });
    let result = await p1;

    await e1.join();

    arktest.assertEQ(EXPECTED_VALUE, result);
}

function testOfPrioritySetting(): void {
    let e = new EAWorker();
    arktest.assertEQ(e.getPriority(), WorkerPriority.PRIORITY_HIGH);
    e.setPriority(WorkerPriority.PRIORITY_HIGH);
    arktest.assertEQ(e.getPriority(), WorkerPriority.PRIORITY_HIGH);
    e.start();
    arktest.assertEQ(e.getPriority(), WorkerPriority.PRIORITY_HIGH);
    e.quit();
}

function main(): int {
    const suite = new arktest.ArkTestsuite('EAWorker Test Suite');

    suite.addAsyncTest('testACoroutineEaworker', testACoroutineEaworker);
    suite.addAsyncTest('testAJCoroutineEaworker', testAJCoroutineEaworker);
    suite.addAsyncTest('testACAJcoroutine', testACAJcoroutine);
    suite.addAsyncTest('testACoroutineCallAJCoroutine', testACoroutineCallAJCoroutine);
    suite.addAsyncTest('testAJCoroutineCallACoroutine', testAJCoroutineCallACoroutine);
    suite.addAsyncTest('testNestedEAWorker', testNestedEAWorker);
    suite.addTest('testOfPrioritySetting', testOfPrioritySetting);

    return suite.run();
}
