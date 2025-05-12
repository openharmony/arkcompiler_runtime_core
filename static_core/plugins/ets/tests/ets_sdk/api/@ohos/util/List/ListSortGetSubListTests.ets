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

import {List} from '@ohos.util.List';

function main() {
    const suite = new ArkTestsuite("LinkedList API tests");

    suite.addTest("List sort with different types in ascending order", testSortDifferentTypesAscending);
    suite.addTest("List sort with object in ascending order", testSortObjectAscending);
    suite.addTest("List sort with different types in descending order", testSortDifferentTypesDescending);
    suite.addTest("List sort with object in descending order", testSortObjectDescending);
    suite.addTest("List getSubList", testGetSubList);
    suite.addTest("List getSubList with start index out of range", testGetSubListStartIndexOutOfRange);
    suite.addTest("List getSubList with end index out of range", testGetSubListEndIndexOutOfRange);
    suite.addTest("List getSubList with end index smaller than start index",
        testGetSubListEndIndexSmallerThanStartIndex);
    suite.addTest("List getSubList with change getSubList", testGetSubListWithChange);
    suite.addTest("List getSubList with empty container", testGetSubListEmptyContainer);
    suite.addTest("List getSubList with start index = 0 and end index = length - 1", testGetSubListFullRange);

    return suite.run();
}

function initializeList<T>(list: List<T>, sourceArray: Array<T>) {
    for (let i = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}

class Temp {
    age: number = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber3: number = 3;
    static readonly testNumber5: number = 5;
    static readonly testNumber8: number = 8;
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2: Temp = { age: 8, name: "li" };
    static readonly testObject3: Temp = { age: 12, name: "wang" };
    static readonly testObject4: Temp = { age: 1, name: "liu" };
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly arrayForNumber = Array.from<number>(TestData.forNumber);
    static readonly forString: String[] = ["ssss", "2", "%", "Z"];
    static readonly arrayForString = Array.from<String>(TestData.forString);
    static readonly forBoolean: Boolean[] = [true, false, false, true];
    static readonly arrayForBoolean = Array.from<boolean>(TestData.forBoolean);
    static readonly forObject: Object[] = [[1], [2, 3, 4], [1, 2], [5]];
    static readonly arrayForObject = Array.from<Object>(TestData.forObject);
    static readonly result1: Number[] = [5, 4, 3, 2, 1];
    static readonly arrayResult1 = Array.from<number>(TestData.result1);
    static readonly result6: Number[] = [2, 3, 4];
    static readonly arrayResult6 = Array.from<number>(TestData.result6);
    static readonly result7: Number[] = [2, 3, 4, 5];
    static readonly arrayResult7 = Array.from<number>(TestData.result7);
}

function testSortDifferentTypesAscending() {
    let list1 = new List<number>();
    initializeList(list1, TestData.arrayResult1);
    list1.sort((a: number, b: number) => a - b);
    for (let i: number = 0; i < list1.length; i++) {
        assertEQ(list1.get(i), TestData.arrayForNumber[i], `Element at index ${i} should match`);
    }

    let list2 = new List<string>();
    initializeList(list2, TestData.arrayForString);
    let array2 = list2.convertToArray();
    list2.sort((a: string, b: string) => a.compareTo(b));
    array2.sort((a: string, b: string) => a.compareTo(b));
    for (let i: number = 0; i < list2.length; i++) {
        assertEQ(list2.get(i), array2[i], `Element at index ${i} should match`);
    }

    let list3 = new List<boolean>();
    initializeList(list3, TestData.arrayForBoolean);
    let array3 = list3.convertToArray();
    list3.sort((a: boolean, b: boolean) => (a ? 1 : 0) - (b ? 1 : 0));
    array3.sort((a: boolean, b: boolean) => (a ? 1 : 0) - (b ? 1 : 0));
    for (let i: number = 0; i < list3.length; i++) {
        assertEQ(list3.get(i), array3[i], `Element at index ${i} should match`);
    }
}

function testSortObjectAscending() {
    let list1 = new List<object>();
    initializeList(list1, TestData.arrayForObject);
    let array1 = list1.convertToArray();
    list1.sort((a: object, b: object): number => {
        let sa = new String(a);
        let sb = new String(b);
        return sa.compareTo(sb);
    });
    array1.sort((a: object, b: object): number => {
        let sa = new String(a);
        let sb = new String(b);
        return sa.compareTo(sb);
    });
    for (let i: number = 0; i < list1.length; i++) {
        assertEQ(list1.get(i), array1[i], `Element at index ${i} should match`);
    }

    let list2 = new List<Temp>();
    list2.add(TestData.testObject1);
    list2.add(TestData.testObject2);
    list2.add(TestData.testObject3);
    list2.add(TestData.testObject4);
    let array2 = list2.convertToArray();
    list2.sort((a: Temp, b: Temp) => a.age - b.age);
    array2.sort((a: Temp, b: Temp) => a.age - b.age);
    for (let i: number = 0; i < list2.length; i++) {
        assertEQ(list2.get(i), array2[i], `Element at index ${i} should match`);
    }
}

function testSortDifferentTypesDescending() {
    let list1 = new List<number>();
    initializeList(list1, TestData.arrayForNumber);
    list1.sort((a: number, b: number) => b - a);
    for (let i: number = 0; i < list1.length; i++) {
        assertEQ(list1.get(i), TestData.arrayResult1[i], `Element at index ${i} should match`);
    }

    let list2 = new List<string>();
    initializeList(list2, TestData.arrayForString);
    let array2 = list2.convertToArray();
    list2.sort((a: string, b: string) => b.compareTo(a));
    array2.sort((a: string, b: string) => b.compareTo(a));
    for (let i: number = 0; i < list2.length; i++) {
        assertEQ(list2.get(i), array2[i], `Element at index ${i} should match`);
    }

    let list3 = new List<boolean>();
    initializeList(list3, TestData.arrayForBoolean);
    let array3 = list3.convertToArray();
    list3.sort((a: boolean, b: boolean) => (b ? 1 : 0) - (a ? 1 : 0));
    array3.sort((a: boolean, b: boolean) => (b ? 1 : 0) - (a ? 1 : 0));
    for (let i: number = 0; i < list3.length; i++) {
        assertEQ(list3.get(i), array3[i], `Element at index ${i} should match`);
    }
}

function testSortObjectDescending() {
    let list1 = new List<object>();
    initializeList(list1, TestData.arrayForObject);
    let array1 = list1.convertToArray();
    list1.sort((a: object, b: object): number => {
        let sa = new String(a);
        let sb = new String(b);
        return sb.compareTo(sa);
    });
    array1.sort((a: object, b: object): number => {
        let sa = new String(a);
        let sb = new String(b);
        return sb.compareTo(sa);
    });
    for (let i: number = 0; i < list1.length; i++) {
        assertEQ(list1.get(i), array1[i], `Element at index ${i} should match`);
    }

    let list2 = new List<Temp>();
    list2.add(TestData.testObject1);
    list2.add(TestData.testObject2);
    list2.add(TestData.testObject3);
    list2.add(TestData.testObject4);
    let array2 = list2.convertToArray();
    list2.sort((a: Temp, b: Temp) => b.age - a.age);
    array2.sort((a: Temp, b: Temp) => b.age - a.age);
    for (let i: number = 0; i < list2.length; i++) {
        assertEQ(list2.get(i), array2[i], `Element at index ${i} should match`);
    }
}

function testGetSubList() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber);
    let getSubList = list.getSubList(TestData.testNumber1, TestData.testNumber5 - TestData.testNumber1);
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber[i], `Element at index ${i} should match`);
    }
    for (let i: number = 0; i < getSubList.length; i++) {
        assertEQ(getSubList.get(i), TestData.arrayResult6[i], `Element at index ${i} should match`);
    }
}

function testGetSubListStartIndexOutOfRange() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber);
    try {
        list.getSubList(-TestData.testNumber1, TestData.testNumber5);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of fromIndex or toIndex is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertFalse(false, "The result should be 'RangeError'");
}

function testGetSubListEndIndexOutOfRange() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber);
    try {
        list.getSubList(TestData.testNumber1, TestData.testNumber8);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of fromIndex or toIndex is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertFalse(false, "The result should be 'RangeError'");
}

function testGetSubListEndIndexSmallerThanStartIndex() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber);
    try {
        list.getSubList(TestData.testNumber5, TestData.testNumber1);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of fromIndex or toIndex is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertFalse(false, "The result should be 'RangeError'");
}

function testGetSubListWithChange() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber);
    let getSubList = list.getSubList(TestData.testNumber1, TestData.testNumber5 - TestData.testNumber1);
    getSubList.add(TestData.testNumber5);
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber[i], `Element at index ${i} should match`);
    }
    for (let i: number = 0; i < getSubList.length; i++) {
        assertEQ(getSubList.get(i), TestData.arrayResult7[i], `Element at index ${i} should match`);
    }
}

function testGetSubListEmptyContainer() {
    let list = new List<number>();
    try {
        list.getSubList(TestData.testNumber1, TestData.testNumber3);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of fromIndex or toIndex is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertFalse(false, "The result should be 'RangeError'");
}

function testGetSubListFullRange() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber);
    let getSubList = list.getSubList(0, TestData.testNumber5 - TestData.testNumber1);
    for (let i: number = 0; i < getSubList.length; i++) {
        assertEQ(getSubList.get(i), list.get(i), `Element at index ${i} should match`);
    }
    assertEQ(getSubList.length, TestData.testNumber5 - TestData.testNumber1, "SubList length should be 4");
}
