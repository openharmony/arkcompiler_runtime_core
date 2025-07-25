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

    suite.addTest("ArrayList insert", testInsertAtIndex);
    suite.addTest("ArrayList insert with index 0 and length", testInsertAtStartAndEnd);
    suite.addTest("ArrayList insert with negative index", testInsertWithNegativeIndex);
    suite.addTest("ArrayList insert with double index", testInsertWithDoubleIndex);
    suite.addTest("ArrayList insert with double index 2", testInsertWithDoubleIndex2);
    suite.addTest("ArrayList has", testHasElement);
    suite.addTest("ArrayList has with inexistent element", testHasNonExistentElement);
    suite.addTest("ArrayList has with every type", testHasVariousTypes);
    suite.addTest("ArrayList getIndexOf with unique element", testGetIndexOfUniqueElement);
    suite.addTest("ArrayList getIndexOf with repeated element", testGetIndexOfRepeatedElement);
    suite.addTest("ArrayList getIndexOf with inexistent element", testGetIndexOfNonExistentElement);

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
    static readonly testNumber8: number = 8;
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
    static readonly result2: Number[] = [1, 2, 3, 3, 4, 5];
    static readonly result3: Number[] = [3, 1, 2, 3, 4, 5, 3];
    static readonly result4: Number[] = [3, 1, 2, 3, 4, 5];
}

function testInsertAtIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.insert(TestData.testNumber3, TestData.testNumber3);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result2[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber6, "ArrayList length should be 6 after insert operation");
}

function testInsertAtStartAndEnd() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.insert(TestData.testNumber3, 0);
    arrayList.insert(TestData.testNumber3, TestData.testNumber6);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result3[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber5 + TestData.testNumber2,
        "ArrayList length should be 7 after insert operations");
}

function testInsertWithNegativeIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= 5. Received value is: -1") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    let exceptionCheck1 = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= 5. Received value is: 8") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList.insert(TestData.testNumber3, -TestData.testNumber1)}, exceptionCheck);
    expectThrow(() => {arrayList.insert(TestData.testNumber3, TestData.testNumber8)}, exceptionCheck1);
}

function testInsertWithDoubleIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.insert(TestData.testNumber3, TestData.testDouble1);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result4[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber6, "ArrayList length should be 6 after insert operation");
}

function testInsertWithDoubleIndex2() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.insert(TestData.testNumber3, TestData.testDouble2);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result4[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber6, "ArrayList length should be 6 after insert operation");
}

function testHasElement() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let result: boolean = arrayList.has(TestData.testNumber3);
    assertTrue(result, "ArrayList should contain the element");
}

function testHasNonExistentElement() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let result: boolean = arrayList.has(TestData.testNumber6);
    assertFalse(result, "ArrayList should not contain the element");
}

function testHasVariousTypes() {
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
    assertTrue(arrayList.has(TestData.testString1), "Elements should be added");
    assertTrue(arrayList.has(TestData.testNumber1), "Elements should be added");
    assertTrue(arrayList.has(TestData.testObject1), "Elements should be added");
    assertTrue(arrayList.has(undefined), "Elements should be added");
    assertTrue(arrayList.has(null), "Elements should be added");
    assertTrue(arrayList.has(true), "Elements should be added");
    assertTrue(arrayList.has(TestData.testObject2), "Elements should be added");
    assertTrue(arrayList.has(TestData.testString2), "Elements should be added");
    assertTrue(arrayList.has(TestData.testString3), "Elements should be added");
    assertTrue(arrayList.has(TestData.testString4), "Elements should be added");
    assertTrue(arrayList.has(TestData.testString5), "Elements should be added");

    arrayList.remove(TestData.testString1);
    arrayList.remove(TestData.testNumber1);
    arrayList.remove(TestData.testObject1);
    arrayList.remove(undefined);
    arrayList.remove(null);
    arrayList.remove(true);
    arrayList.remove(TestData.testObject2);
    arrayList.remove(TestData.testString2);
    arrayList.remove(TestData.testString3);
    arrayList.remove(TestData.testString4);
    arrayList.remove(TestData.testString5);
    assertFalse(arrayList.has(TestData.testString1), "Elements should be removed");
    assertFalse(arrayList.has(TestData.testNumber1), "Elements should be removed");
    assertFalse(arrayList.has(TestData.testObject1), "Elements should be removed");
    assertFalse(arrayList.has(undefined), "Elements should be removed");
    assertFalse(arrayList.has(null), "Elements should be removed");
    assertFalse(arrayList.has(true), "Elements should be removed");
    assertFalse(arrayList.has(TestData.testObject2), "Elements should be removed");
    assertFalse(arrayList.has(TestData.testString2), "Elements should be removed");
    assertFalse(arrayList.has(TestData.testString3), "Elements should be removed");
    assertFalse(arrayList.has(TestData.testString4), "Elements should be removed");
    assertFalse(arrayList.has(TestData.testString5), "Elements should be removed");
}

function testGetIndexOfUniqueElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    arrayList.add(TestData.testNumber2);
    assertEQ(arrayList.getIndexOf(TestData.testNumber1), 0, "Index of first element should be 0");
    assertEQ(arrayList.getIndexOf(TestData.testNumber2), TestData.testNumber6, "Index of last element should be 6");
}

function testGetIndexOfRepeatedElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    assertEQ(arrayList.getIndexOf(TestData.testNumber3), TestData.testNumber1, "Index of element should be 1");
}

function testGetIndexOfNonExistentElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(TestData.testNumber3);
    }
    assertEQ(arrayList.getIndexOf(TestData.testNumber5), -TestData.testNumber1,
        "Index of non-existent element should be -1");
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
