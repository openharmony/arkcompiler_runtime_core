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

function main(): int {
    let failures: int = 0;
    failures += check((): int => { return length() }, "length");
    failures += check((): int => { return shiftUnshift() }, "shiftUnshift");
    failures += check((): int => { return pushPop() }, "pushPop");
    return check(failures, "All tests run");
}

class Handler<T> extends DefaultArrayProxyHandler<T> {}
function newProxy<T>(a: Array<T>): Array<T> {

    return Proxy.create<T>(a, new Handler<T>())
}

function check<T>(newArrayFn: () => Array<T>, testFn: (a: Array<T>) => int): int {
    let failures = 0

    failures += testFn(newArrayFn())
    failures += testFn(newProxy(newArrayFn()))

    return failures
}

function length(): int {
    return check(() => new Array<number>, (a: Array<number>) => {
        if (a.length != 0) {
            return fail;
        }
        let threw = false;
        try {
            a.length = 12;
        } catch (e) {
            threw = true;
        }
        if (!threw) {
            return fail;
        }
        if (a.length != 0) {
            return fail;
        }

        a.pushECMA(1, 2, 3, 4, 5);
        if (a.length != 5) {
            return fail;
        }
        a.length = 3
        if (a.length != 3) {
            return fail
        }
        if (a[0] != 1 || a[1] != 2 || a[2] != 3) {
            return fail;
        }
        a.length = 0
        if (a.length != 0) {
            return fail
        }
        return success;
    })
}

function shiftUnshift(): int {
    return check(() => new Array<number>, (a: Array<number>) => {
        if (a.shift() !== undefined) {
            return fail;
        }
        a.unshift(3, 2, 1)
        if (a.length != 3) {
            return fail;
        }
        a.unshift(6, 5, 4)
        if (a.length != 6) {
            return fail;
        }
        for (let i = 5; i >= 0; i--) {
            if (a.shift()! != i + 1) {
                return fail
            }
            if (a.length != i) {
                return fail;
            }
        }
        return success;
    })
}

function pushPop(): int {
    return check(() => new Array<number>, (a: Array<number>) => {
        if (a.pop() !== undefined) {
            return fail;
        }
        a.pushECMA(1, 2, 3)
        if (a.length != 3) {
            return fail;
        }
        a.pushECMA(4, 5, 6)
        if (a.length != 6) {
            return fail;
        }
        for (let i = 5; i >= 0; i--) {
            if (a.pop()! != i + 1) {
                return fail
            }
            if (a.length != i) {
                return fail;
            }
        }
        return success;
    })
}


function check(result: int, message: String): int {
    if (result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    try {
        result = fn();
    } catch(e) {
        console.println("FAILED: " + message + " => Exception catched: " + e);
        return fail;
    }
    if (result == success) {
      return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}
