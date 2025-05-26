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

    suite.addTest("List clear a non-empty container", testClearNonEmptyContainer);
    suite.addTest("List clear an empty container", testClearEmptyContainer);
    suite.addTest("List set multiple types of data in the container", testSetMultipleTypes);
    suite.addTest("List set elements when the container is empty", testSetElementsInEmptyContainer);
    suite.addTest("List set getIndexOf->set->removeByIndex", testSetGetIndexOfRemoveByIndex);
    suite.addTest("List set getIndexOf->set->has", testSetGetIndexOfHas);
    suite.addTest("List set set->getLast", testSetGetLast);
    suite.addTest("List set set->getFirst", testSetGetFirst);
    suite.addTest("List convertToArray with a non-empty container", testConvertToArrayNonEmptyContainer);
    suite.addTest("List convertToArray with an empty container", testConvertToArrayEmptyContainer);
    suite.addTest("List convertToArray with change array", testConvertToArrayChangeArray);

    return suite.run();
}

function initializeList(list: List<number>, sourceArray: Array<number>) {
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
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber4: number = 4;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testString1: string = "string";
    static readonly testString2: string = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2 = [0, 1, null];
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
}

function testClearNonEmptyContainer() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    assertEQ(list.length, TestData.arrayForNumber1.length, "List length should be same as TestData.arrayForNumber1");
    list.clear();
    assertEQ(list.length, 0, "List length should be 0 after clear");
}

function testClearEmptyContainer() {
    let list = new List<number>();
    assertEQ(list.length, 0, "List length should be 0");
    list.clear();
    assertEQ(list.length, 0, "List length should be 0 after clear");
}

function testSetMultipleTypes() {
    let list = new List<number | string | boolean | null | object | undefined>();
    for (let i = 0; i < TestData.testNumber6; i++) {
        list.add(i);
    }

    assertEQ(list.set(0, TestData.testString1), TestData.testString1, "Set successful");
    assertEQ(list.set(TestData.testNumber1, TestData.testNumber3), TestData.testNumber3, "Set successful");
    assertEQ(list.set(TestData.testNumber2, null), null, "Set successful");
    assertEQ(list.set(TestData.testNumber3, true), true, "Set successful");
    assertEQ(list.set(TestData.testNumber4, TestData.testObject2), TestData.testObject2, "Set successful");
    assertEQ(list.set(TestData.testNumber5, undefined), undefined, "Set successful");
}

function testSetElementsInEmptyContainer() {
    let list = new List<number>();
    try {
        list.set(0, TestData.testNumber3);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertFalse(false, "The result should be 'RangeError'");
}

function testSetGetIndexOfRemoveByIndex() {
    let list = new List<number | string | boolean | null | object | undefined>();
    list.add(TestData.testString1);
    list.add(TestData.testNumber1);
    list.add(TestData.testObject1);
    list.add(undefined);
    list.add(null);
    list.add(true);

    let index = list.getIndexOf(TestData.testObject1);
    let elem1 = list.set(Math.floor(index), TestData.testString2);
    let elem2 = list.removeByIndex(Math.floor(index));
    assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testSetGetIndexOfHas() {
    let list = new List<number | string | boolean | null | object | undefined>();
    list.add(TestData.testString1);
    list.add(TestData.testNumber1);
    list.add(TestData.testObject1);
    list.add(undefined);
    list.add(null);
    list.add(true);

    let index = list.getIndexOf(TestData.testObject1);
    let elem = list.set(Math.floor(index), TestData.testString2);
    assertTrue(list.has(elem), "The element set successful");
}

function testSetGetLast() {
    let list = new List<number | string | boolean | null | object | undefined>();
    list.add(TestData.testString1);
    list.add(TestData.testNumber1);
    list.add(TestData.testObject1);
    list.add(undefined);
    list.add(null);
    list.add(true);

    let elem1 = list.set(list.length - TestData.testNumber1, TestData.testString2);
    let elem2 = list.getLast();
    assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testSetGetFirst() {
    let list = new List<number | string | boolean | null | object | undefined>();
    list.add(TestData.testString1);
    list.add(TestData.testNumber1);
    list.add(TestData.testObject1);
    list.add(undefined);
    list.add(null);
    list.add(true);

    let elem1 = list.set(0, TestData.testString2);
    let elem2 = list.getFirst();
    assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testConvertToArrayNonEmptyContainer() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let array = list.convertToArray();
    assertTrue(Array.isArray(array), "The result should be an Array");
    assertEQ(array.length, list.length, "Array length should be same as List length");
    for (let i = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(array[i], list.get(i), `Element at index ${i} should be equal`);
    }
}

function testConvertToArrayEmptyContainer() {
    let list = new List<number>();
    let array = list.convertToArray();
    assertEQ(array.length, 0, "Array length should be 0");
}

function testConvertToArrayChangeArray() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let array = list.convertToArray();
    array.push(TestData.testNumber1);
    assertEQ(array.length, list.length + TestData.testNumber1, "Array length should be List length + 1");
    assertEQ(list.length, TestData.arrayForNumber1.length, "List length should not change");
    for (let i = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber1[i], `Element at index ${i} should be equal`);
    }
}