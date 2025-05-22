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

    suite.addTest("ArrayList convertToArray with empty container", testConvertToArrayWithEmptyContainer);
    suite.addTest("ArrayList convertToArray", testConvertToArray);
    suite.addTest("ArrayList convertToArray with change convertArray", testConvertToArrayWithChange);
    suite.addTest("ArrayList convertToArray and check whether the array is normal", testConvertToArrayAndCheckArray);
    suite.addTest("ArrayList convertToArray and check whether the array is normal",
        testConvertToArrayAndCheckArrayWithCat);
    suite.addTest("ArrayList isEmpty", testIsEmptyAfterAddingElement);
    suite.addTest("ArrayList isEmpty with empty container", testIsEmptyInitially);
    suite.addTest("ArrayList index getter", testIndexGetter);
    suite.addTest("ArrayList index setter", testIndexSetter);
    suite.addTest("ArrayList index out of range", testIndexOutOfRange);
    suite.addTest("ArrayList index with double index", testIndexWithDoubleIndex);
    suite.addTest("ArrayList index with double index 2", testIndexWithDoubleIndex2);
    suite.addTest("ArrayList index about for...of", testIndexForOfLoop);

    return suite.run();
}

class Dog {
    age: number = 0;
    name: string = "";

    constructor(age: number, name: string) {
        this.age = age;
        this.name = name;
    }
}

class Cat {
    age1: number = 0;
    name1: string = "";

    constructor(age: number, name: string) {
        this.age1 = age;
        this.name1 = name;
    }
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber6: number = 6;
    static readonly testNumber8: number = 8;
    static readonly testString1: string = "string";
    static readonly testString4: string = "%";
    static readonly testDouble1 = 0.4;
    static readonly testDouble2 = 0.7;
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly result3: Number[] = [3, 1, 2, 3, 4, 5, 3];
    static readonly result4: Number[] = [3, 1, 2, 3, 4, 5];
}

function testConvertToArrayWithEmptyContainer() {
    let arrayList = new ArrayList<number>();
    let array = arrayList.convertToArray();
    assertEQ(array.length, arrayList.length, "Converted array should have the same length as ArrayList");
    assertEQ(array.length, 0, "Converted array should have length 0");
}

function testConvertToArray() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let array = arrayList.convertToArray();
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(array[i], arrayList[i], `Element at index ${i} should match`);
    }
    assertEQ(array.length, arrayList.length, "Converted array should have the same length as ArrayList");
}

function testConvertToArrayWithChange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.result4);
    let array = arrayList.convertToArray();
    assertTrue(array instanceof Array, "The result of convertToArray should be Array");
    array.push(TestData.testNumber3);
    for (let i: int = 0; i < array.length; i++) {
        assertEQ(array[i], TestData.result3[i], `Element at index ${i} should match`);
    }
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result4[i], `Element at index ${i} should match`);
    }
    assertEQ(array.length, TestData.testNumber6 + TestData.testNumber1,
        "Converted array should have the correct length after modification");
    assertEQ(arrayList.length, TestData.testNumber6,
        "Original ArrayList should remain unchanged after array modification");
}

function testConvertToArrayAndCheckArray() {
    let arrayList = new ArrayList<Dog>();
    let dog = new Dog(TestData.testNumber1, TestData.testString1);
    arrayList.add(dog);
    let array = arrayList.convertToArray();
    array.filter((value: Dog) => {
        assertTrue(value instanceof Dog, "The converted array's filter normally");
        return true;
    })
}

function testConvertToArrayAndCheckArrayWithCat() {
    let arrayList = new ArrayList<Dog | Cat>();
    let dog = new Dog(TestData.testNumber1, TestData.testString1);
    arrayList.add(dog);
    let array = arrayList.convertToArray();
    let cat = new Cat(TestData.testNumber2, TestData.testString4);
    array.fill(cat);
    assertFalse(array[0] instanceof Dog, "The converted array's fill normally");
}

function testIsEmptyAfterAddingElement() {
    let arrayList = new ArrayList<number>();
    arrayList.add(TestData.testNumber3);
    let result: boolean = arrayList.isEmpty();
    assertFalse(result, "ArrayList should not be empty after adding an element");
}

function testIsEmptyInitially() {
    let arrayList = new ArrayList<number>();
    let result: boolean = arrayList.isEmpty();
    assertTrue(result, "ArrayList should be empty initially");
}

function testIndexGetter() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], i + TestData.testNumber1, `Element at index ${i} should match`);
    }
}

function testIndexSetter() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    for (let i: number = 0; i < arrayList.length; i++) {
        arrayList[i] = TestData.testNumber3;
    }
    let arrayForEach = new Array<number>();
    arrayList.forEach((value: number) => {
        arrayForEach.push(value);
    });
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(arrayForEach[i], TestData.testNumber3, `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, arrayForEach.length, "ArrayList length should match arrayForEach length");
}

function testIndexOutOfRange() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            `The value of \"index\" is out of range. It must be >= 0 && <= 4. Received value is: 8`) &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    expectThrow(() => {arrayList[TestData.testNumber8]}, exceptionCheck);
}

function testIndexWithDoubleIndex() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    assertEQ(arrayList[TestData.testDouble1], arrayList[0], "Element mismatch during index operation with double index");
}

function testIndexWithDoubleIndex2() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    assertEQ(arrayList[TestData.testDouble2], arrayList[0], "Element mismatch during index operation with double index");
}

function testIndexForOfLoop() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    let elem: number = TestData.testNumber1;
    for (let item of arrayList) {
        assertEQ(item, elem++, "Element mismatch during iteration");
    }
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
