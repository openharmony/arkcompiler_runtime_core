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

class MyPayload {
    public s: string;
    constructor(a: number) {
        this.s = `MyPayload.s=${a}`;
    }
}

const l0 = () => `l0`;
const l1 = (a0: string) => `l1: ${a0}`;
const l2 = (a0: string, a1: number, a2: MyPayload) => `l2: ${a0} ${a1} ${a2.s}`;

function f0() {
    return `f0`;
}

function f1(a0: string) {
    return `f1: ${a0}`;
}

function f2(a0: string, a1: number, a2: MyPayload) {
    return `f2: ${a0} ${a1} ${a2.s}`;
}

class C {
    static s0() {
        return `s0`;
    }

    static s1(a0: string) {
        return `s1: ${a0}`;
    }

    static s2(a0: string, a1: number, a2: MyPayload) {
        return `s2: ${a0} ${a1} ${a2.s}`;
    }
}

/**
 * This test verifies launching lambdas with and without arguments.
 * This function calls launch to execute l0, l1, and l2 functions, and verifies that their return values match expectations.
 */
async function testLaunchLambdas(): Promise<void> {
    // Launch l0 function with no arguments
    const l0Result = await launch(l0);

    // Launch l1 function with one string argument 'l1_arg_0'
    const l1Result = await launch<string>() {
        return l1('l1_arg_0')
    };

    // Launch l2 function with three arguments: string, number, and a custom object
    const l2ResultPromise: Promise<string> = launch<string>() {
        return l2('l2_arg_0', 2.73, new MyPayload(3.14))
    };

    const l2Result = await l2ResultPromise;

    // Assert that the results match the expected values
    arktest.assertEQ(l0Result, 'l0');
    arktest.assertEQ(l1Result, 'l1: l1_arg_0');
    arktest.assertEQ(l2Result, "l2: l2_arg_0 2.73 MyPayload.s=3.14");
}

/**
 * This test checks launching regular functions with and without arguments
 * This function calls launch to execute f0, f1, and f2 functions, and verifies their return values.
 */
async function testLaunchFunctions(): Promise<void> {
    // Launch f0 function with no arguments
    const f0ResultPromise: Promise<string> = launch(f0);
    const f0Result = await f0ResultPromise;

    // Launch f1 function with one string argument 'f1_arg_0'
    const f1ResultPromise: Promise<string> = launch<string>() {
        return f1('f1_arg_0')
    };
    const f1Result = await f1ResultPromise;

    // Launch f2 function with three arguments: string, number, and a custom object
    const f2ResultPromise: Promise<string> = launch<string>() {
        return f2('f2_arg_0', 2.73, new MyPayload(3.14))
    };

    const f2Result = await f2ResultPromise;

    // Assert that the results match the expected values
    arktest.assertEQ(f0Result, 'f0');
    arktest.assertEQ(f1Result, 'f1: f1_arg_0');
    arktest.assertEQ(f2Result, 'f2: f2_arg_0 2.73 MyPayload.s=3.14');
}

/**
 * Tests the behavior of the launch function when launching static methods.
 * This function calls launch to execute C.s0, C.s1, and C.s2 methods, and verifies their return values.
 */
async function testLaunchStatic(): Promise<void> {
    // Launch C.s0 static method with no arguments
    const s0Result = await launch(C.s0);

    // Launch C.s1 static method with one string argument 'C.s1_arg_0'
    const s1Result = await launch<string>() {
        return C.s1('C.s1_arg_0')
    };

    // Launch C.s2 static method with three arguments: string, number, and a custom object
    const s2ResultPromise: Promise<string> = launch<string>() {
        return C.s2('C.s2_arg_0', 2.73, new MyPayload(3.14))
    };

    const s2Result = await s2ResultPromise;

    // Assert that the results match the expected values
    arktest.assertEQ(s0Result, 's0');
    arktest.assertEQ(s1Result, 's1: C.s1_arg_0');
    arktest.assertEQ(s2Result, 's2: C.s2_arg_0 2.73 MyPayload.s=3.14');
}


async function main(): Promise<int> {
    let testSuite = new arktest.ArkTestsuite('coroutines.launch');
    testSuite.addAsyncTest('testLaunchLambdas', testLaunchLambdas);
    testSuite.addAsyncTest('testLaunchFunctions', testLaunchFunctions);
    testSuite.addAsyncTest('testLaunchStatic', testLaunchStatic);
    let res = await testSuite.runAsync();
    return res;
}
