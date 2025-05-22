/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import {ArrayList} from "@ohos.util.ArrayList";
import {BusinessError} from "@ohos.base";

const OutOfBoundsErrorCodeId: number = 10200001;

function main() {
    const suite = new ArkTestsuite("ArrayList API tests");

    suite.addTest("ArrayList remove with unique element", testRemoveUniqueElement);
    suite.addTest("ArrayList remove with repeated element", testRemoveRepeatedElement);
    suite.addTest("ArrayList remove with inexistent element", testRemoveNonExistentElement);
    suite.addTest("ArrayList removeByRange", testRemoveByRange);
    suite.addTest("ArrayList removeByRange with start index out of range", testRemoveByRangeStartIndexOutOfRange);
    suite.addTest("ArrayList removeByRange with end index out of range", testRemoveByRangeEndIndexOutOfRange);
    suite.addTest("ArrayList removeByRange with end index smaller than start index",
        testRemoveByRangeEndIndexSmallerThanStartIndex);
    suite.addTest("ArrayList removeByRange with empty container", testRemoveByRangeEmptyContainer);
    suite.addTest("ArrayList removeByRange with start index = end index", testRemoveByRangeStartIndexEqualsEndIndex);
    suite.addTest("ArrayList removeByRange with start index = 0 and end index = length - 1",
        testRemoveByRangeFullRange);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testInt4: int = 4;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testNumber8: number = 8;
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly result7: Number[] = [2, 3, 4, 5];
}

function testRemoveUniqueElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    arrayList.add(TestData.testNumber2);
    arrayList.remove(TestData.testNumber1);
    arrayList.remove(TestData.testNumber2);
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.testNumber3, `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber5, "ArrayList length should be 5 after remove operation");
}

function testRemoveRepeatedElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    arrayList.remove(TestData.testNumber3);
    for (let i: number = TestData.testNumber1; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.testNumber3, `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber5, "ArrayList length should be 5 after remove operation");
}

function testRemoveNonExistentElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    let array = arrayList.convertToArray();
    let result: boolean = arrayList.remove(TestData.testNumber6);
    assertFalse(result, "Remove operation should return false for non-existent element");
    assertEQ(arrayList.length, TestData.testNumber6, "ArrayList length should remain unchanged");
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], array[i], `Element at index ${i} should match`);
    }
}

function testRemoveByRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.removeByRange(0, TestData.testNumber1);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result7[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber5 - TestData.testNumber1,
        "ArrayList length should be 4 after removeByRange operation");
}

function testRemoveByRangeStartIndexOutOfRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"fromIndex\" is out of range. It must be >= 0 && <= 2. Received value is: -1") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.removeByRange(-TestData.testNumber1, TestData.testNumber3)}, exceptionCheck);
}

function testRemoveByRangeEndIndexOutOfRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"toIndex\" is out of range. It must be >= 0 && <= 5. Received value is: 8") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.removeByRange(TestData.testNumber1, TestData.testNumber8)}, exceptionCheck);
}

function testRemoveByRangeEndIndexSmallerThanStartIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"fromIndex\" is out of range. It must be >= 0 && <= 0. Received value is: 3") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.removeByRange(TestData.testNumber3, TestData.testNumber1)}, exceptionCheck);
}

function testRemoveByRangeEmptyContainer() {
    let arrayList = new ArrayList<number>();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message == "Container is empty") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.removeByRange(0, TestData.testNumber1)}, exceptionCheck);
}

function testRemoveByRangeStartIndexEqualsEndIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"fromIndex\" is out of range. It must be >= 0 && <= 2. Received value is: 3") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.removeByRange(TestData.testNumber3, TestData.testNumber3)}, exceptionCheck);
}

function testRemoveByRangeFullRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.removeByRange(0, TestData.testNumber5);
    assertEQ(arrayList.length, 0, "ArrayList length should be 0 after removeByRange operation");
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
