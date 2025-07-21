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

function main(): int {
    const suite = new ArkTestsuite("RegExpExecArray pop() Tests");

    suite.addTest("pop() returns the last element", testPopReturnsLast);
    suite.addTest("pop() reduces length", testPopReducesLength);
    suite.addTest("Multiple pop() calls remove elements in reverse order", testPopMultipleTimes);
    suite.addTest("pop() on empty array returns undefined", testPopEmptyReturnsUndefined);
    suite.addTest("$_get", testGet);
    suite.addTest("$_set", testSet);

    return suite.run();
}

/**
 * Test: pop() should return the last element in result array.
 */
function testPopReturnsLast(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    let popped = match.pop();
    assertEQ(popped, "c", "testPopReturnsLast");
}

/**
 * Test: pop() should reduce result array length.
 */
function testPopReducesLength(): void {
    let match = new RegExpExecArray(0, "abc", ["a", "b", "c"]);
    match.pop();
    assertEQ(match.result.length, 2, "testPopReducesLength");
}

/**
 * Test: multiple pop() calls should remove elements in reverse order.
 */
function testPopMultipleTimes(): void {
    let match = new RegExpExecArray(0, "abc", ["x", "y", "z"]);
    assertEQ(match.pop(), "z", "testPopMultipleTimes step 1");
    assertEQ(match.pop(), "y", "testPopMultipleTimes step 2");
    assertEQ(match.pop(), "x", "testPopMultipleTimes step 3");
}

/**
 * Test: pop() on empty result should return undefined.
 */
function testPopEmptyReturnsUndefined(): void {
    let match = new RegExpExecArray(0, "abc", []);
    let popped = match.pop();
    assertEQ(popped, undefined, "testPopEmptyReturnsUndefined");
}

/**
 * Test: $_get test.
 */
function testGet(): void {
    let match = new RegExpExecArray(0, "abc", ["x", "y", "z"]);

    assertEQ(match[0], "x", "testGet, int index, step 1");
    assertEQ(match[0 as number], "x", "testGet, number index, step 1");

    assertEQ(match[1], "y", "testGet, int index, step 2");
    assertEQ(match[1 as number], "y", "testGet, number index, step 2");

    assertEQ(match[2], "z", "testGet, int index, step 3");
    assertEQ(match[2 as number], "z", "testGet, number index, step 3");

    expectError((): void => { match[-1] }, new ArrayIndexOutOfBoundsError("idx = -1; length = 3"))
    expectError((): void => { match[match.length] }, new ArrayIndexOutOfBoundsError("idx = 3; length = 3"))
}

/**
 * Test: $_set test.
 */
function testSet(): void {
    let match = new RegExpExecArray(0, "abc", ["x", "y", "z"]);

    match[0] = "x1"
    assertEQ(match[0], "x1", "testSet, int index, step 1");
    assertEQ(match[0 as number], "x1", "testSet, number index, step 1");

    match[1] = "y1"
    assertEQ(match[1], "y1", "testSet, int index, step 2");
    assertEQ(match[1 as number], "y1", "testSet, number index, step 2");

    match[2] = "z1"
    assertEQ(match[2], "z1", "testSet, int index, step 3");
    assertEQ(match[2 as number], "z1", "testSet, number index, step 3");

    expectError((): void => { match[-1] = "a" }, new ArrayIndexOutOfBoundsError("idx = -1; length = 3"))
    expectError((): void => { match[match.length] = "b" }, new ArrayIndexOutOfBoundsError("idx = 3; length = 3"))
}
