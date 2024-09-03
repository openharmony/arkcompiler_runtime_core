/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
    failures += check((): int => { return testReadonlyArrayGet()}, "ReadonlyArray $_get");
    failures += check((): int => { return testReadonlyArrayGetInvalidIndex()}, "ReadonlyArray $_get Negative Index");
    failures += check((): int => { return testArrayLikeGet()}, "ArrayLike $_get");
    failures += check((): int => { return testArrayLikeGetInvalidIndex()}, "ArrayLike $_get Negative Index");
    failures += check((): int => { return testConcatArrayGet()}, "ConcatArray $_get");
    failures += check((): int => { return testConcatArrayGetInvalidIndex()}, "ConcatArray $_get Negative Index");

    return check(failures, "All tests run");
}


const arr: Number[] = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];

function testReadonlyArrayGet(): int {
    let a = Array.from<number>(arr);
    let b = a as ReadonlyArray<number>

    return b[2] == 237 ? success : fail;
}

function testReadonlyArrayGetInvalidIndex(): int {
    let a = Array.from<number>(arr);
    let b = a as ReadonlyArray<number>
    try {
        return b[-1] == undefined ? success : fail;
    } catch (e) {
        if (e instanceof RangeError) {
            return success;
        }
    }
    return fail;
}

function testArrayLikeGet(): int {
    let a = Array.from<number>(arr);
    let b = a as ArrayLike<number>

    return b[2] == 237 ? success : fail;
}

function testArrayLikeGetInvalidIndex(): int {
    let a = Array.from<number>(arr);
    let b = a as ArrayLike<number>
    try {
        return b[-1] == undefined ? success : fail;
    } catch (e) {
        if (e instanceof RangeError) {
            return success;
        }
    }
    return fail;
}

function testConcatArrayGet(): int {
    let a = Array.from<number>(arr);
    let b = a as ConcatArray<number>

    return b[2] == 237 ? success : fail;
}

function testConcatArrayGetInvalidIndex(): int {
    let a = Array.from<number>(arr);
    let b = a as ConcatArray<number>
    try {
        return b[-1] == undefined ? success : fail;
    } catch (e) {
        if (e instanceof RangeError) {
            return success;
        }
    }
    return fail;
}

function check(result: int, message: String): int {
    if(result == success) {
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
    if(result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}
