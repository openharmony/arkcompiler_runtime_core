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

function main() {
    const suite = new ArkTestsuite("ArrayList API tests");

    suite.addTest("ArrayList increaseCapacityTo with bigger value", testIncreaseCapacityToBiggerValue);
    suite.addTest("ArrayList increaseCapacityTo with smaller value", testIncreaseCapacityToSmallerValue);
    suite.addTest("ArrayList trimToCurrentLength", testTrimToCurrentLength);
    suite.addTest("ArrayList iterator with empty container", testIteratorEmptyContainer);
    suite.addTest("ArrayList iterator", testIterator);
    suite.addTest("ArrayList API composition testing", testAPIComposition);
    suite.addTest("ArrayList the add, insert, and removeByIndex APIs are mixed", testAddInsertRemoveByIndex);
    suite.addTest("ArrayList the getIndexOf, removeByIndex, and has APIs are mixed", testGetIndexOfRemoveByIndexHas1);
    suite.addTest("ArrayList the getIndexOf, removeByIndex, and has APIs are mixed", testGetIndexOfRemoveByIndexHas2);

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
    static readonly testNumber6: number = 6;
    static readonly testNumber10: number = 10;
    static readonly testNumber12: number = 12;
    static readonly testNumber18: number = 18;
    static readonly testNumber1w: number = 10000;
    static readonly testString1: string = "string";
    static readonly testString2: string = "";
    static readonly testString3: string = "`";
    static readonly testString4: string = "%";
    static readonly testString5: string = `"`;
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2 = [0, 1, 2, undefined];
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly result1: Number[] = [5, 4, 3, 2, 1];
    static readonly arrayResult8 = `{"DEFAULT_CAPACITY":10,"capacity":10,"buffer":["string",1,` +
        `{"age":10,"name":"zhang"},null,null,true,[0,1,2,null]]}`;
}

function testIncreaseCapacityToBiggerValue() {
    let arrayList = new ArrayList<number>();
    for (let i: number = 0; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    let cap: number = arrayList.getCapacity();
    arrayList.increaseCapacityTo(TestData.testNumber12);
    let cap1: number = arrayList.getCapacity();
    for (let i: int = 0; i < TestData.testNumber3; i++) {
        arrayList.add(TestData.forNumber[i]);
    }
    let cap2: number = arrayList.getCapacity();
    initializeArrayList(arrayList, TestData.forNumber);
    assertEQ(cap, TestData.testNumber10, "Initial ArrayList capacity should be 10");
    assertEQ(cap1, TestData.testNumber12, "ArrayList capacity should be 12 after increaseCapacityTo");
    assertEQ(cap2, TestData.testNumber12, "ArrayList capacity should remain 12 after adding elements");
    assertEQ(arrayList.getCapacity(), TestData.testNumber18,
        "ArrayList capacity should be 18 after adding more elements");

    arrayList.increaseCapacityTo(TestData.testNumber1w);
    assertEQ(arrayList.getCapacity(), TestData.testNumber1w,
        "ArrayList capacity should be 10000 after increaseCapacityTo");
}

function testIncreaseCapacityToSmallerValue() {
    let arrayList = new ArrayList<number>();
    for (let i: number = 1; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    let cap: number = arrayList.getCapacity();
    arrayList.increaseCapacityTo(TestData.testNumber2);
    assertEQ(cap, TestData.testNumber10, "Initial ArrayList capacity should be 10");
    assertEQ(arrayList.getCapacity(), TestData.testNumber10,
        "ArrayList capacity should remain unchanged when increasing to a smaller value");
}

function testTrimToCurrentLength() {
    let arrayList = new ArrayList<number>();
    for (let i: number = 1; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    let cap: number = arrayList.getCapacity();
    arrayList.trimToCurrentLength();
    assertEQ(cap, TestData.testNumber10, "Initial ArrayList capacity should be 10");
    assertEQ(arrayList.getCapacity(), arrayList.length, "ArrayList capacity should be trimmed to current length");
}

function testIteratorEmptyContainer() {
    let arrayList = new ArrayList<number>();
    let elem: number = 0;
    for (let item of arrayList) {
        elem = TestData.testNumber1;
    }
    let iter = arrayList.$_iterator();
    let temp: IteratorResult<number> = iter.next();
    while (!temp.done) {
        elem = TestData.testNumber1;
    }
    assertEQ(elem, 0, "Iterator over empty ArrayList should not iterate any elements");
}

function testIterator() {
    let arrayList = new ArrayList<number>();
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    let elem1: number = TestData.testNumber1;
    for (let item of arrayList) {
        assertEQ(item, elem1++, "Element mismatch during iteration");
    }
    let iter = arrayList.$_iterator();
    let temp: IteratorResult<number> = iter.next();
    let elem2: number = TestData.testNumber1;
    while (!temp.done) {
        assertEQ(temp.value, elem2++, "Element mismatch during iteration");
        temp = iter.next();
    }
    assertEQ(elem1, TestData.testNumber6, "Iterator should iterate all elements correctly");
    assertEQ(elem2, TestData.testNumber6, "Iterator should iterate all elements correctly");
}

function testAPIComposition() {
    extendTestInterfaceComposition();

    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.result1);
    let array = arrayList.convertToArray();
    arrayList.sort();
    array.sort();
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], array[i], `Element at index ${i} should match`);
    }

    arrayList.subArrayList(TestData.testNumber1, TestData.testNumber3);
    arrayList.clone();
    assertEQ(arrayList.getCapacity(), TestData.testNumber10, "ArrayList capacity should be updated correctly");
    arrayList.increaseCapacityTo(TestData.testNumber12);
    assertEQ(arrayList.getCapacity(), TestData.testNumber12, "ArrayList capacity should be updated correctly");
    arrayList.trimToCurrentLength();
    assertEQ(arrayList.getCapacity(), arrayList.length, "ArrayList capacity should be updated correctly");

    arrayList.replaceAllElements((): number => {
        return TestData.testNumber3;
    });

    arrayList.clear();
    let elem: number = 0;
    for (let item of arrayList) {
        elem = TestData.testNumber1;
    }
    assertEQ(elem, 0, "Iterator over empty ArrayList should not iterate any elements");
}

function testAddInsertRemoveByIndex() {
    let arrayList = new ArrayList<number | string | Array<number | string | boolean> | boolean | undefined | null |
        Object>();
    arrayList.add(TestData.testString1);
    arrayList.add(TestData.testNumber1);
    arrayList.add(TestData.testObject1);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    arrayList.insert(TestData.testString1, 0);
    arrayList.add(TestData.testObject2);
    arrayList.removeByIndex(0);
    assertEQ(JSON.stringify(arrayList), TestData.arrayResult8, "Element should be equal");
}

function testGetIndexOfRemoveByIndexHas1() {
    let arrayList = new ArrayList<number | string>();
    arrayList.add(TestData.testString1);
    arrayList.insert(TestData.testString2, 0);
    arrayList.insert(TestData.testString3, arrayList.length);
    let startIndex = arrayList.getIndexOf(TestData.testString2);
    arrayList.removeByIndex(startIndex);
    let endIndex = arrayList.getIndexOf(TestData.testString3);
    arrayList.removeByIndex(endIndex);
    assertFalse(arrayList.has(TestData.testString2), "Elements should be removed");
    assertFalse(arrayList.has(TestData.testString3), "Elements should be removed");
}

function testGetIndexOfRemoveByIndexHas2() {
    let arrayList = new ArrayList<number | string>();
    arrayList.insert(TestData.testString3, 0);
    arrayList.insert(TestData.testString4, TestData.testNumber1);
    arrayList.insert(TestData.testString5, TestData.testNumber2);
    let index1 = arrayList.getIndexOf(TestData.testString3);
    arrayList.removeByIndex(index1);
    let index2 = arrayList.getIndexOf(TestData.testString4);
    arrayList.removeByIndex(index2);
    let index3 = arrayList.getIndexOf(TestData.testString5);
    arrayList.removeByIndex(index3);
    assertFalse(arrayList.has(TestData.testString3), "The element should be removed");
    assertFalse(arrayList.has(TestData.testString4), "The element should be removed");
    assertFalse(arrayList.has(TestData.testString5), "The element should be removed");
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}

function extendTestInterfaceComposition() {
    let arrayList = new ArrayList<number | string | boolean | undefined | null | Object>();
    assertEQ(arrayList.length, 0, "Initial ArrayList length should be 0");

    let result: boolean = arrayList.add(TestData.testNumber3);
    assertTrue(result, "ArrayList add should return true");

    arrayList.insert(TestData.testNumber1, 0);
    result = arrayList.has(TestData.testNumber3);
    assertTrue(result, "ArrayList should contain the element");

    let startIndex = arrayList.getIndexOf(TestData.testNumber1);
    arrayList.removeByIndex(startIndex);
    assertFalse(arrayList.has(TestData.testNumber1), "Elements should be removed");

    arrayList.remove(TestData.testNumber3);
    arrayList.add(TestData.testString1);
    arrayList.add(TestData.testNumber1);
    arrayList.add(TestData.testObject1);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    let endIndex = arrayList.getLastIndexOf(true);
    arrayList.removeByIndex(endIndex);
    assertFalse(arrayList.has(true), "Elements should be removed");

    arrayList.removeByRange(0, arrayList.length - 1);
    assertFalse(arrayList.isEmpty(), "ArrayList should not be empty");
}
