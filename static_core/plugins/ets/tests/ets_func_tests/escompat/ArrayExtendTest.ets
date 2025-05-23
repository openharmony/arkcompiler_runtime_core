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
    const suite = new ArkTestsuite("Array API tests for SDK");
    suite.addTest("Array extendTo with negative length ", testExtendToNegLength);
    suite.addTest("Array extendTo with positivie length ", testExtendToPosLength);
    suite.addTest("Array extendTo incorrect length", testExtendToError);
    return suite.run();
}

class TestData {
    static readonly src = Array.of<number>(1, 2, 3);
    static readonly arrayLength1: number = -1;
    static readonly initialValue1: number = -1;
    static readonly expected1 = Array.from<number>(TestData.src);
    static readonly arrayLength2: number = 5;
    static readonly initialValue2: number = -1;
    static readonly expected2 = Array.of<number>(1, 2, 3, -1, -1);
    static readonly arrayLength3: number = 2147483648;
    static readonly initialValue3: number = -1;
}

function testExtendToNegLength() {
    let a = Array.from<number>(TestData.src);
    a.extendTo(TestData.arrayLength1, TestData.initialValue1);
    let result: boolean = equaled(TestData.expected1, a);
    assertTrue(result, "Error. Got: " + a + ". Expected: " + TestData.expected1);
}

function testExtendToPosLength() {
    let a = Array.from<number>(TestData.src);
    a.extendTo(TestData.arrayLength2, TestData.initialValue2);
    let result: boolean = equaled(TestData.expected2, a);
    assertTrue(result, "Error. Got: " + a + ". Expected: " + TestData.expected2);
}

function testExtendToError() {
    let a = Array.from<number>(TestData.src);
    expectError(
        () => { a.extendTo(TestData.arrayLength3, TestData.initialValue3) },
        new RangeError("arrayLength must be <= int32 max")
    )
}

function equaled(arr1:  Array<number>, arr2:  Array<number>): boolean {
    if (arr1.length != arr2.length) {
        return false;
    }
    for (let i = 0; i < arr1.length; i++) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }
    return true;
}
