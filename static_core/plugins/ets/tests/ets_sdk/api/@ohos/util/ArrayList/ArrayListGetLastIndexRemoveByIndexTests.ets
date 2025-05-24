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

    suite.addTest("ArrayList getLastIndexOf with unique element", testGetLastIndexOfUniqueElement);
    suite.addTest("ArrayList getLastIndexOf with repeated element", testGetLastIndexOfRepeatedElement);
    suite.addTest("ArrayList getLastIndexOf with inexistent element", testGetLastIndexOfNonExistentElement);
    suite.addTest("ArrayList removeByIndex", testRemoveByIndex);
    suite.addTest("ArrayList removeByIndex with 0 and length - 1", testRemoveByIndexWithBoundaryIndices);
    suite.addTest("ArrayList removeByIndex with out of range index", testRemoveByIndexOutOfRange);
    suite.addTest("ArrayList removeByIndex with double index", testRemoveByIndexWithDoubleIndex);
    suite.addTest("ArrayList removeByIndex with double index 2", testRemoveByIndexWithDoubleIndex2);
    suite.addTest("ArrayList removeByIndex with empty container", testRemoveByIndexEmptyContainer);
    suite.addTest("ArrayList removeByIndex with the same index", testRemoveByIndexSameIndex);

    return suite.run();
}

class Temp {
    age: number = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testNumber12: number = 12;
    static readonly testString1: string = "string";
    static readonly testString2: string = "";
    static readonly testString3: string = "`";
    static readonly testString4: string = "%";
    static readonly testString5: string = `"`;
    static readonly testDouble1 = 0.4;
    static readonly testDouble2 = 0.7;
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2 = [0, 1, 2, undefined];
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly result5: Number[] = [1, 2, 3, 5];
    static readonly result6: Number[] = [2, 3, 4];
    static readonly result7: Number[] = [2, 3, 4, 5];
}

function testGetLastIndexOfUniqueElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    arrayList.add(TestData.testNumber2);
    assertEQ(arrayList.getLastIndexOf(TestData.testNumber1), 0, "Last index of first element should be 0");
    assertEQ(arrayList.getLastIndexOf(TestData.testNumber2), TestData.testNumber6,
        "Last index of last element should be 6");
}

function testGetLastIndexOfRepeatedElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    assertEQ(arrayList.getLastIndexOf(TestData.testNumber3), TestData.testNumber5, "Last index of element should be 5");
}

function testGetLastIndexOfNonExistentElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    assertEQ(arrayList.getLastIndexOf(TestData.testNumber5), -TestData.testNumber1,
        "Last index of non-existent element should be -1");
}

function testRemoveByIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.removeByIndex(TestData.testNumber3);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result5[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber5 - TestData.testNumber1,
        "ArrayList length should be 4 after removeByIndex operation");
}

function testRemoveByIndexWithBoundaryIndices() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.removeByIndex(0);
    arrayList.removeByIndex(TestData.testNumber3);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result6[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber3, "ArrayList length should be 3 after removeByIndex operations");
}

function testRemoveByIndexOutOfRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            `The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: 12`) &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    let exceptionCheck1 = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            `The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: -12`) &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.removeByIndex(TestData.testNumber12)}, exceptionCheck);
    expectThrow(() => {arrayList.removeByIndex(-TestData.testNumber12)}, exceptionCheck1);
}

function testRemoveByIndexWithDoubleIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.removeByIndex(TestData.testDouble1);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result7[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber5 - TestData.testNumber1,
        "ArrayList length should be 4 after removeByIndex operation");
}

function testRemoveByIndexWithDoubleIndex2() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.removeByIndex(TestData.testDouble2);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result7[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber5 - TestData.testNumber1,
        "ArrayList length should be 4 after removeByIndex operation");
}

function testRemoveByIndexEmptyContainer() {
    let arrayList = new ArrayList<number>();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message == `Container is empty`) &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.removeByIndex(0)}, exceptionCheck);
}

function testRemoveByIndexSameIndex() {
    let arrayList = new ArrayList<number | string | Array<number | string | boolean> | boolean | undefined | null |
        Object>();
    arrayList.add(TestData.testString1);
    arrayList.add(TestData.testNumber1);
    arrayList.add(TestData.testObject1);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    arrayList.add(TestData.testObject2);
    arrayList.add(TestData.testString2);
    arrayList.add(TestData.testString3);
    arrayList.add(TestData.testString4);
    arrayList.add(TestData.testString5);
    let array = arrayList.convertToArray();
    let index1 = arrayList.getIndexOf(TestData.testString2);
    let result = arrayList.removeByIndex(index1);
    let result1 = arrayList.removeByIndex(index1);
    assertNE(result, result1, "The elements removed twice are different");

    let index2 = arrayList.getIndexOf(TestData.testString4);
    let result2 = arrayList.removeByIndex(index2);
    let result3 = arrayList.removeByIndex(index2);
    assertNE(result2, result3, "The elements removed twice are different");
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], array[i], `Element at index ${i} should match`);
    }
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
