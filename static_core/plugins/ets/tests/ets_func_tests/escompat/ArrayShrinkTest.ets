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
    suite.addTest("Array shrinkTo with negative length ", testShrinkToNegLength);
    suite.addTest("Array shrinkTo with normal length ", testShrinkToLength);
    suite.addTest("Array shrinkTo with out length ", testShrinkToOutLength);
    suite.addTest("Array shrinkTo with Int.MIN_VALUE", testShrinkToIntMinValue);
    return suite.run();
}

class TestData {
    static readonly src = Array.of<number>(1, 2, 3);
    static readonly arrayLength1: number = -1;
    static readonly expected1 = Array.from<number>(TestData.src);
    static readonly arrayLength2: number = 2;
    static readonly expected2 = TestData.src.slice(0, 2);
    static readonly arrayLength3: number = 5;
    static readonly expected3 = Array.from<number>(TestData.src);
    static readonly arrayLength4: number = Int.MIN_VALUE as number;
    static readonly limit4: number = 10;
    static readonly expected4 = new Array<number>;
}

function testShrinkToNegLength() {
    let a = Array.from<number>(TestData.src);
    a.shrinkTo(TestData.arrayLength1);
    let result: boolean = equaled(TestData.expected1, a);
    assertTrue(result, "Error. Got: " + a + ". Expected: " + TestData.expected1);
}

function testShrinkToLength() {
    let a = Array.from<number>(TestData.src);
    a.shrinkTo(TestData.arrayLength2);
    let result: boolean = equaled(TestData.expected2, a);
    assertTrue(result, "Error. Got: " + a + ". Expected: " + TestData.expected2);
}

function testShrinkToOutLength() {
    let a = Array.from<number>(TestData.src);
    a.shrinkTo(TestData.arrayLength3);
    let result: boolean = equaled(TestData.expected3, a);
    assertTrue(result, "Error. Got: " + a + ". Expected: " + TestData.expected3);
}

function testShrinkToIntMinValue() {
    let a = Array.from<number>(TestData.src);
    for (let i: number = 0; i < TestData.limit4; ++i) {
        a.shrinkTo(TestData.arrayLength4 + i);
        let result: boolean = equaled(TestData.expected4, a);
        assertTrue(result, "Error. " + i + " Got: " + a + ". Expected: " + TestData.expected4);
    }
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
