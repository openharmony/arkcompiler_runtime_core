/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

let passed = false;

function reset() {
    passed = false;
}

function check(test: String) {
    if (!passed) {
        console.println("Test " + test + " failed");
        throw new Error();
    }
}

function noarg(): Object | null {
    passed = true;
    return null;
}

function onearg(x: int): Object | null {
    if (x != 1) {
        console.println("Test 'onearg': expected argument '1' but was " + x);
        throw new Error();
    }
    passed = true;
    return null;
}

function twoargs(x: int, y: int): Object | null {
    if (x != 1) {
        console.println("Test 'twoargs': expected first argument '1' but was " + x);
        throw new Error();
    }
    if (y != 2) {
        console.println("Test 'twoargs': expected second argument '2' but was " + y);
        throw new Error();
    }
    passed = true;
    return null;
}

function threeargs(x: int, y: int, z: int): Object | null {
    if (x != 1) {
        console.println("Test 'threeargs': expected first argument '1' but was " + x);
        throw new Error();
    }
    if (y != 2) {
        console.println("Test 'threeargs': expected second argument '2' but was " + y);
        throw new Error();
    }
    if (z != 3) {
        console.println("Test 'threeargs': expected third argument '3' but was " + z);
        throw new Error();
    }
    passed = true;
    return null;
}

function fourargs(x1: int, x2: int, x3: int, x4: int): Object | null {
    if (x1 != 1) {
        console.println("Test 'fourargs': expected first argument '1' but was " + x1);
        throw new Error();
    }
    if (x2 != 2) {
        console.println("Test 'fourargs': expected second argument '2' but was " + x2);
        throw new Error();
    }
    if (x3 != 3) {
        console.println("Test 'fourargs': expected third argument '3' but was " + x3);
        throw new Error();
    }
    if (x4 != 4) {
        console.println("Test 'fourargs': expected fourth argument '4' but was " + x4);
        throw new Error();
    }
    passed = true;
    return null;
}

function fiveargs(x1: int, x2: int, x3: int, x4: int, x5: int): Object | null {
    if (x1 != 1) {
        console.println("Test 'fiveargs': expected first argument '1' but was " + x1);
        throw new Error();
    }
    if (x2 != 2) {
        console.println("Test 'fiveargs': expected second argument '2' but was " + x2);
        throw new Error();
    }
    if (x3 != 3) {
        console.println("Test 'fiveargs': expected third argument '3' but was " + x3);
        throw new Error();
    }
    if (x4 != 4) {
        console.println("Test 'fiveargs': expected fourth argument '4' but was " + x4);
        throw new Error();
    }
    if (x5 != 5) {
        console.println("Test 'fiveargs': expected fifth argument '5' but was " + x5);
        throw new Error();
    }
    passed = true;
    return null;
}

export function main(): int {
    // test launch.short instruction
    reset()
    launch noarg();
    Coroutine.Schedule();
    check("noarg");

    reset()
    launch onearg(1);
    Coroutine.Schedule();
    check("onearg");

    reset()
    launch twoargs(1, 2);
    Coroutine.Schedule();
    check("twoargs");

    // test launch
    reset()
    launch threeargs(1, 2, 3);
    Coroutine.Schedule();
    check("threeargs");

    reset()
    launch fourargs(1, 2, 3, 4);
    Coroutine.Schedule();
    check("fourargs");

    // test launch.range
    reset()
    launch fiveargs(1, 2, 3, 4, 5);
    Coroutine.Schedule();
    check("fiveargs");
    return 0;
}
