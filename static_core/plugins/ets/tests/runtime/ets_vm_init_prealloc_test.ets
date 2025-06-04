/*
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

package PreallocTest;

function fillHeap(arr: Object[], numberOfIter: number): void
{
    let globalIdx = 0;
    const megabyte = Math.pow(2, 20);

    arr[globalIdx++] = new Object[megabyte];
    arr[globalIdx++] = new Object[megabyte / 2];
    arr[globalIdx++] = new Object[megabyte / 32];
    arr[globalIdx++] = new Object[megabyte / 64];
    arr[globalIdx++] = new Object[megabyte / 128];
    arr[globalIdx++] = new Object[megabyte / 256];

    let newIterNmb = numberOfIter - globalIdx;
    for (let idx = 0; idx < newIterNmb; ++idx) {
        arr[globalIdx++] = new Object;
    }

    throw new Error("ets_vm_init_prealloc_test.ets: unsuccessful attempt of fill the heap.");
}

function compareExceptionObjects(
    error: Error,
    expectedExceptionName: string,
    expectedExceptionMsg: string,
    expectedExceptionStack: string
): boolean
{
    if (error.name !== expectedExceptionName) {
        console.log(`Expected '${expectedExceptionName}' exception name, actual '${error.name}'`);
        return false;
    }

    if (error.message !== expectedExceptionMsg) {
        console.log(`Expected '${expectedExceptionMsg}' exception message, actual '${error.message}'`);
        return false;
    }

    if (error.stack !== expectedExceptionStack) {
        console.log(`Expected '${expectedExceptionStack}' exception stack, but actual '${error.stack}'`);
        return false;
    }

    let expectedErrorToString = error.name + ((error.name !== "" && error.message)? ": " + error.message: error.message);
    if (error.toString() !== expectedErrorToString) {
        console.log(`Expected '${expectedErrorToString}' exception stack, but actual '${error.toString()}'`);
        return false;
    }

    return true;
}

function testThrowingPreallocOOMObjectSuite(): Error
{
    let numberOfIter = 900000;
    let arr = new Object[numberOfIter];

    let error: Error;
    try {
        fillHeap(arr, numberOfIter);
    } catch(e: OutOfMemoryError) {
        return e;
    }

    return new Error();
}

function testThrowingPreallocOOMObject(
    expectedExceptionName: string,
    expectedExceptionMsg: string,
    expectedExceptionStack: string
): boolean
{
    let error = testThrowingPreallocOOMObjectSuite();

    GC.startGC(GC.FULL_CAUSE);

    return compareExceptionObjects(error, expectedExceptionName, expectedExceptionMsg, expectedExceptionStack);
}

function testThrowingPreallocOOMObjectTwiceSuite(): Error
{
    let numberOfIter = 900000;
    let arr = new Object[numberOfIter];
    let arr2: Object;

    let error: Error = new Error("testThrowingPreallocOOMObjectTwiceSuite failed");
    try {
        fillHeap(arr, numberOfIter);
    } catch(e: OutOfMemoryError) {
    }

    try {
        // Try to allocate more memory to get one more OOM
        arr2 = new Object[numberOfIter];
        fillHeap(arr2, numberOfIter);
    } catch(e: OutOfMemoryError) {
        error = e;
    }

    return error;
}

function testThrowingPreallocOOMObjectTwice(
    expectedExceptionName: string,
    expectedExceptionMsg: string,
    expectedExceptionStack: string
): boolean
{
    let error = testThrowingPreallocOOMObjectTwiceSuite();

    GC.startGC(GC.FULL_CAUSE);

    return compareExceptionObjects(error, expectedExceptionName, expectedExceptionMsg, expectedExceptionStack);
}
