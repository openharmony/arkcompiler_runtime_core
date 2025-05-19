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
