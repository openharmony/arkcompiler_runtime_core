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

    suite.addTest("LinkedList insert an element within the container's length range", testInsertWithinRange);
    suite.addTest("LinkedList insert an element at the boundary of the container", testInsertAtBoundary);
    suite.addTest("LinkedList insert insert->addFirst->getIndexOf->removeByIndex->has",
        testInsertAddFirstGetIndexOfRemoveByIndexHas);
    suite.addTest("LinkedList insert elements at index > length", testInsertIndexGreaterThanLength);
    suite.addTest("LinkedList insert elements at index < 0", testInsertIndexLessThanZero);
    suite.addTest("LinkedList has an element in the container", testHasElementInContainer);
    suite.addTest("LinkedList has an element not in the container", testHasElementNotInContainer);
    suite.addTest("LinkedList has with every type", testHasEveryType);
    suite.addTest("LinkedList get with an index within the container's length range", testGetWithinRange);
    suite.addTest("LinkedList get with an index outside the container's length range", testGetOutsideRange);

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
    static readonly testNumber20: number = 20;
    static readonly testNumber1w: number = 10000;
    static readonly testString1: string = "string";
    static readonly testString2: string = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2 = [0, 1, null];
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
}

function testInsertWithinRange() {
    let linkedList = new LinkedList<number | string | boolean | null | object | undefined>();
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testNumber2);
    linkedList.insert(TestData.testNumber1, TestData.testNumber1);
    linkedList.insert(TestData.testNumber2, TestData.testString1);
    linkedList.insert(TestData.testNumber3, true);
    linkedList.insert(TestData.testNumber4, null);
    linkedList.insert(TestData.testNumber5, TestData.testObject1);
    linkedList.insert(TestData.testNumber6, undefined);

    assertEQ(linkedList.length, TestData.testNumber8, "LinkedList length should be 8");
    assertEQ(linkedList.get(0), TestData.testNumber1, "Element at index 0 should match");
    assertEQ(linkedList.get(TestData.testNumber1), TestData.testNumber1, "Element at index 6 should match");
    assertEQ(linkedList.get(TestData.testNumber2), TestData.testString1, "Element at index 1 should match");
    assertEQ(linkedList.get(TestData.testNumber3), true, "Element at index 2 should match");
    assertEQ(linkedList.get(TestData.testNumber4), null, "Element at index 3 should match");
    assertEQ(linkedList.get(TestData.testNumber5), TestData.testObject1, "Element at index 4 should match");
    assertEQ(linkedList.get(TestData.testNumber6), undefined, "Element at index 5 should match");
    assertEQ(linkedList.get(TestData.testNumber7), TestData.testNumber2, "Element at index 7 should match");
}

function testInsertAtBoundary() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    linkedList.insert(TestData.arrayForNumber1.length, 0);
    linkedList.insert(0, 0);

    for (let i: number = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(linkedList.get(i + TestData.testNumber1), TestData.arrayForNumber1[i], "Element should match");
    }
    assertEQ(linkedList.get(0), 0, "Element should be 0");
    assertEQ(linkedList.get(linkedList.length - TestData.testNumber1), 0, "Element should be 0");
    assertEQ(linkedList.length, TestData.arrayForNumber1.length + TestData.testNumber2, "LinkedList length should + 2");
}

function testInsertAddFirstGetIndexOfRemoveByIndexHas() {
    let linkedList = new LinkedList<number | string>();
    linkedList.insert(0, TestData.testNumber1);
    assertEQ(linkedList.get(0), TestData.testNumber1, "Element should be 0");

    linkedList.addFirst(TestData.testString2);
    let index = linkedList.getIndexOf(TestData.testString2);
    assertEQ(index, 0, "The index of the first element should be 0");

    linkedList.removeByIndex(index);
    assertFalse(linkedList.has(TestData.testString2), "The element should be removed");
}

function testInsertIndexGreaterThanLength() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    try {
        linkedList.insert(TestData.testNumber6, TestData.testNumber1);
    } catch (e) {
        assertTrue(e instanceof RangeError, "Throw error when LinkedList index out of range");
        return;
    }
    assertFalse(false, "Throw error when LinkedList index out of range");
}

function testInsertIndexLessThanZero() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    try {
        linkedList.insert(-TestData.testNumber6, TestData.testNumber1);
    } catch (e) {
        assertTrue(e instanceof RangeError, "Throw error when LinkedList index out of range");
        return;
    }
    assertFalse(false, "Throw error when LinkedList index out of range");
}

function testHasElementInContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertTrue(linkedList.has(TestData.testNumber1), "LinkedList should contain value 1");
}

function testHasElementNotInContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertFalse(linkedList.has(TestData.testNumber1w), "LinkedList should not contain value 10000");
}

function testHasEveryType() {
    let linkedList = new LinkedList<number | string | boolean | null | object | undefined>();

    linkedList.add(TestData.testString2);
    assertEQ(linkedList.getLast(), TestData.testString2, "The element should be present");

    linkedList.add(TestData.testString1);
    assertEQ(linkedList.getLast(), TestData.testString1, "The element should be present");

    linkedList.add(TestData.testNumber1);
    assertEQ(linkedList.getLast(), TestData.testNumber1, "The element should be present");

    linkedList.add(null);
    assertEQ(linkedList.getLast(), null, "The element should be present");

    linkedList.add(true);
    assertEQ(linkedList.getLast(), true, "The element should be present");

    linkedList.add(TestData.testObject2);
    assertEQ(linkedList.getLast(), TestData.testObject2, "The element should be present");
}

function testGetWithinRange() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.get(0), 0, "Element at index 0 should be 0");
    assertEQ(linkedList.get(TestData.testNumber1), TestData.testNumber1, "Element at index 1 should be 1");
    assertEQ(linkedList.get(TestData.arrayForNumber1.length - TestData.testNumber1), TestData.testNumber20,
        "Element at index 20 should be 20");
}

function testGetOutsideRange() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.get(TestData.testNumber1w), undefined, "Element at index 10000 should be undefined");
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}