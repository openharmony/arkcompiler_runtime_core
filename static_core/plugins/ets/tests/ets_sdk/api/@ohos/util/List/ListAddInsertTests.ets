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
    const suite = new ArkTestsuite("List API tests");

    suite.addTest("List add elements of multiple types to the container", testAddMultipleTypes);
    suite.addTest("List add special characters to the container", testAddSpecialCharacters);
    suite.addTest("List insert an element within the container's length range", testInsertWithinRange);
    suite.addTest("List insert an element at the boundary of the container", testInsertAtBoundary);
    suite.addTest("List insert insert->addFirst->getIndexOf->removeByIndex->has",
        testInsertGetIndexOfRemoveByIndexHas);
    suite.addTest("List insert elements at index > length", testInsertIndexGreaterThanLength);
    suite.addTest("List insert elements at index < 0", testInsertIndexLessThanZero);

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
    static readonly testNumber4: number = 4;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testNumber7: number = 7;
    static readonly testNumber8: number = 8;
    static readonly testString1: string = "string";
    static readonly testString2: string = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
}

function testAddMultipleTypes() {
    let list = new List<number | string | boolean | null | object | undefined>();
    list.add(TestData.testNumber1);
    list.add(TestData.testString1);
    list.add(true);
    list.add(null);
    list.add(TestData.testObject1);
    list.add(undefined);

    assertEQ(list.length, TestData.testNumber6, "List length should be 6");
    assertEQ(list.get(0), TestData.testNumber1, "Element at index 0 should match");
    assertEQ(list.get(TestData.testNumber1), TestData.testString1, "Element at index 1 should match");
    assertEQ(list.get(TestData.testNumber2), true, "Element at index 2 should match");
    assertEQ(list.get(TestData.testNumber3), null, "Element at index 3 should match");
    assertEQ(list.get(TestData.testNumber4), TestData.testObject1, "Element at index 4 should match");
    assertEQ(list.get(TestData.testNumber5), undefined, "Element at index 5 should match");
}

function testAddSpecialCharacters() {
    let list = new List<string>();
    list.add(TestData.testString2);
    assertEQ(list.get(0), TestData.testString2, "Add successful");
    assertEQ(list.length, TestData.testNumber1, "List length should be 1");
}

function testInsertWithinRange() {
    let list = new List<number | string | boolean | null | object | undefined>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.insert(TestData.testNumber1, TestData.testNumber1);
    list.insert(TestData.testString1, TestData.testNumber2);
    list.insert(true, TestData.testNumber3);
    list.insert(null, TestData.testNumber4);
    list.insert(TestData.testObject1, TestData.testNumber5);
    list.insert(undefined, TestData.testNumber6);

    assertEQ(list.length, TestData.testNumber8, "List length should be 8");
    assertEQ(list.get(0), TestData.testNumber1, "Element at index 0 should match");
    assertEQ(list.get(TestData.testNumber1), TestData.testNumber1, "Element at index 6 should match");
    assertEQ(list.get(TestData.testNumber2), TestData.testString1, "Element at index 1 should match");
    assertEQ(list.get(TestData.testNumber3), true, "Element at index 2 should match");
    assertEQ(list.get(TestData.testNumber4), null, "Element at index 3 should match");
    assertEQ(list.get(TestData.testNumber5), TestData.testObject1, "Element at index 4 should match");
    assertEQ(list.get(TestData.testNumber6), undefined, "Element at index 5 should match");
    assertEQ(list.get(TestData.testNumber7), TestData.testNumber2, "Element at index 7 should match");
}

function testInsertAtBoundary() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    list.insert(0, TestData.arrayForNumber1.length);
    list.insert(0, 0);

    for (let i: number = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(list.get(i + TestData.testNumber1), TestData.arrayForNumber1[i], "Element should match");
    }
    assertEQ(list.get(0), 0, "Element should be 0");
    assertEQ(list.get(list.length - TestData.testNumber1), 0, "Element should be 0");
    assertEQ(list.length, TestData.arrayForNumber1.length + TestData.testNumber2, "List length should + 2");
}

function testInsertGetIndexOfRemoveByIndexHas() {
    let list = new List<number | string>();
    list.insert(TestData.testString2, 0);
    let index = list.getIndexOf(TestData.testString2);
    assertEQ(index, 0, "The index of the first element should be 0");
    list.removeByIndex(index);
    assertFalse(list.has(TestData.testString2), "The element should be removed");
}

function testInsertIndexGreaterThanLength() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    try {
        list.insert(TestData.testNumber1, TestData.testNumber6);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertFalse(false, "The result should be 'RangeError'");
}

function testInsertIndexLessThanZero() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    try {
        list.insert(-TestData.testNumber1, TestData.testNumber6);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertFalse(false, "The result should be 'RangeError'");
}

function initializeList(list: List<number>, sourceArray: Array<number>) {
    for (let i = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}