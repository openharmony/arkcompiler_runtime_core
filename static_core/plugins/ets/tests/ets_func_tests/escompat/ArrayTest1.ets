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

const success : int = 0;
const fail    : int = 1;

const withoutProxy  : int = 0;
const withProxy     : int = 1;
let testPhase       : int = withoutProxy;

function runTestPhase(): int {
    let failures: int = 0;
    failures += check((): int => { return createDefault() }, "Create default empty Array");
    failures += check((): int => { return createWithGivenZeroSize() }, "Create Array with given zero size");
    failures += check((): int => { return createWithGivenSize() }, "Create Array with given size");
    failures += check((): int => { return createWithNegativeInitialCapacity() }, "Create Array with negative capacity");
    failures += check((): int => { return isArrayRawArrayNumber() }, "isArrayRawArrayNumber");
    failures += check((): int => { return isArrayArrayNumber() }, "isArrayArrayNumber");
    failures += check((): int => { return isArrayNullUndefined() }, "isArrayNullUndefined");
    failures += check((): int => { return push() }, "push");
    failures += check((): int => { return pushECMA() }, "pushECMA");
    failures += check((): int => { return setNull() }, "setNull");
    failures += check((): int => { return setCustomClass() }, "setCustomClass");
    failures += check((): int => { return toSplicedArg2() }, "toSplicedArg2");
    failures += check((): int => { return toSplicedArg1() }, "toSplicedArg1");
    return failures;
}

function main(): int {
    let res = success;
    if (runTestPhase() != success) {
        console.println("FAILURE without Proxy.");
        res = fail;
    }
    testPhase = withProxy;
    if (runTestPhase() != success) {
        console.println("FAILURE with Proxy.");
        res = fail;
    }
    return res;
}

const d: Array<Number> = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];
const dd: FixedArray<Number> = [1, 0, 3, 4, 0, 6];

class Handler<T> extends DefaultArrayProxyHandler<T> {}
function newProxy<T>(a: Array<T>): Array<T> {

    return Proxy.create<T>(a, new Handler<T>())
}

function check<T>(newArrayFn: () => Array<T>, testFn: (a: Array<T>) => int): int {
    let failures = 0

    if (testPhase == withoutProxy) {
        failures += testFn(newArrayFn())
    } else {
        failures += testFn(newProxy(newArrayFn()))
    }

    return failures
}

function createDefault(): int {
    return check(() => new Array<Number>(), (arr: Array<Number>) => {
        if (!(arr instanceof Array)) {
            throw new Error("arr is not instance of Array.");
        }
        assertEQ(arr.length, 0);
        return success;
    })
}

function createWithNegativeInitialCapacity(): int {
    try {
        new Array<Number>(-1);
    } catch (e) {
        if (e instanceof NegativeArraySizeError) {
            return success;
        }
        throw new Error(`Expects NegativeArraySizeError to be thrown, but actual exception: '${e}'`);
    }
    throw new Error("Expects NegativeArraySizeError to be thrown, but no exception in actual.");
}

function createWithGivenZeroSize(): int {
    return check(() => new Array<Number>(0), (arr: Array<Number>) => {
        assertEQ(arr.length, 0);
        return success;
    })
}

function createWithGivenSize(): int {
    const ARRAY_SIZE = 10;
    return check(() => new Array<Number>(ARRAY_SIZE), (arr: Array<Number>) => {
        assertEQ(arr.length, ARRAY_SIZE);
        return success;
    })
}

function isArrayRawArrayNumber(): int {
    let n: FixedArray<Number> = [];
    assertTrue(Array.isArray(n));
    return success;
}

function isArrayArrayNumber(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        assertTrue(Array.isArray(arr));
        return success;
    })
}

function isArrayNullUndefined(): int {
    assertFalse(Array.isArray(null));
    assertFalse(Array.isArray(undefined));
    return success;
}

function push(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i])
            assertEQ(arr.at(i), d[i]);
        }
        assertEQ(arr.length, d.length);
        return success;
    })
}

function pushECMA(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        arr.pushECMA(...d);
        assertEQ(arr.length, d.length);
        for (let i: int = 0; i < d.length; ++i) {
            assertEQ(arr.at(i), d[i]);
        }
        return success;
    })
}

function setNull(): int {
    return check(() => new Array<string | null>(3), (arr: Array<string | null>) => {
        arr[1] = "abc"
        arr[2] = null;
        assertEQ(arr[2], null);
        return success;
    })
}

class A {}

function setCustomClass() : int {
    return check(() => new Array<A>(1), (arr: Array<A>) => {
        arr[0] = new A()
        return success
    })
}

function toSplicedArg1(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < dd.length; ++i) {
            arr.push(dd[i]);
        }
        let newArr = arr.toSpliced(1);
        assertEQ(newArr.at(0), 1);
        assertEQ(newArr.length, 1);
        assertEQ(arr.length, dd.length);
        for (let i = 0; i < dd.length; i++) {
            assertEQ(arr[i], dd[i]);
        }
        return success;
    })
}

function toSplicedArg2(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < dd.length; ++i) {
            arr.push(dd[i]);
        }
        let newArr = arr.toSpliced(1, 2);
        assertEQ(newArr.length, 4);
        assertEQ(newArr[0], 1);
        assertEQ(newArr[1], 4);
        assertEQ(newArr[2], 0);
        assertEQ(newArr[3], 6);
        assertEQ(arr.length, dd.length);
        for (let i = 0; i < dd.length; i++) {
            assertEQ(arr[i], dd[i]);
        }
        return success;
    })
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    // try {
        result = fn();
    // } catch(e) {
    //     console.println("FAILED: " + message + " => Exception catched: " + e);
    //     return fail;
    // }
    if (result == success) {
        return success;
    }
    console.println("FAILED: " + message);
    return fail;
}
