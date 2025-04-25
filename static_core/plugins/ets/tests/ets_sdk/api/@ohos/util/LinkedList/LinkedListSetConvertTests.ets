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

import {LinkedList} from "@ohos.util.LinkedList";

function main() {
    const suite = new ArkTestsuite("LinkedList API tests");

    suite.addTest("LinkedList set multiple types of data in the container", testSetMultipleTypes);
    suite.addTest("LinkedList set elements when the container is empty", testSetElementsInEmptyContainer);
    suite.addTest("LinkedList set getIndexOf->set->removeByIndex", testSetGetIndexOfRemoveByIndex);
    suite.addTest("LinkedList set getIndexOf->set->has", testSetGetIndexOfHas);
    suite.addTest("LinkedList set set->getLast", testSetGetLast);
    suite.addTest("LinkedList set set->getFirst", testSetGetFirst);
    suite.addTest("LinkedList convertToArray with a non-empty container", testConvertToArrayNonEmptyContainer);
    suite.addTest("LinkedList convertToArray with an empty container", testConvertToArrayEmptyContainer);
    suite.addTest("LinkedList convertToArray with change array", testConvertToArrayChangeArray);

    return suite.run();
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
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

function testSetMultipleTypes() {
    let linkedList = new LinkedList<number | string | boolean | null | object | undefined>();
    for (let i = 0; i < TestData.testNumber6; i++) {
        linkedList.add(i);
    }

    assertEQ(linkedList.set(0, TestData.testString1), TestData.testString1, "Set successful");
    assertEQ(linkedList.set(TestData.testNumber1, TestData.testNumber3), TestData.testNumber3, "Set successful");
    assertEQ(linkedList.set(TestData.testNumber2, null), null, "Set successful");
    assertEQ(linkedList.set(TestData.testNumber3, true), true, "Set successful");
    assertEQ(linkedList.set(TestData.testNumber4, TestData.testObject2), TestData.testObject2, "Set successful");
    assertEQ(linkedList.set(TestData.testNumber5, undefined), undefined, "Set successful");
}

function testSetElementsInEmptyContainer() {
    let linkedList = new LinkedList<number>();
    try {
        linkedList.set(0, TestData.testNumber1);
    } catch (e) {
        assertEQ(e instanceof RangeError, true, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.", "The result should be 'Index out of bounds'");
    }
}

function testSetGetIndexOfRemoveByIndex() {
    let linkedList = initMultiTypeLinkedList();

    let index = linkedList.getIndexOf(TestData.testObject1);
    let elem1 = linkedList.set(Math.floor(index), TestData.testString2);
    let elem2 = linkedList.removeByIndex(Math.floor(index));
    assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testSetGetIndexOfHas() {
    let linkedList = initMultiTypeLinkedList();

    let index = linkedList.getIndexOf(TestData.testObject1);
    let elem = linkedList.set(Math.floor(index), TestData.testString2);
    assertTrue(linkedList.has(elem), "The element set successful");
}

function testSetGetLast() {
    let linkedList = initMultiTypeLinkedList();

    let elem1 = linkedList.set(linkedList.length - TestData.testNumber1, TestData.testString2);
    let elem2 = linkedList.getLast();
    assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testSetGetFirst() {
    let linkedList = initMultiTypeLinkedList();

    let elem1 = linkedList.set(0, TestData.testString2);
    let elem2 = linkedList.getFirst();
    assertEQ(elem1, elem2, "The result should be the same for both operations");
}

function testConvertToArrayNonEmptyContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let array = linkedList.convertToArray();
    assertTrue(Array.isArray(array), "The result should be an Array");
    assertEQ(array.length, linkedList.length, "Array length should be same as LinkedList length");
    for (let i = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(array[i], linkedList.get(i), `Element at index ${i} should be equal`);
    }
}

function testConvertToArrayEmptyContainer() {
    let linkedList = new LinkedList<number>();
    let array = linkedList.convertToArray();
    assertEQ(array.length, 0, "Array length should be 0");
}

function testConvertToArrayChangeArray() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let array = linkedList.convertToArray();
    array.push(TestData.testNumber1);
    assertEQ(array.length, linkedList.length + TestData.testNumber1, "Array length should be LinkedList length + 1");
    assertEQ(linkedList.length, TestData.arrayForNumber1.length,
        "LinkedList length should not change");
    for (let i = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber1[i], `Element at index ${i} should be equal`);
    }
}

function initMultiTypeLinkedList() {
    let linkedList = new LinkedList<number | string | boolean | null | object | undefined>();
    linkedList.add(TestData.testString1);
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testObject1);
    linkedList.add(undefined);
    linkedList.add(null);
    linkedList.add(true);
    
    return linkedList;
}