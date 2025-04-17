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

    suite.addTest("LinkedList removeByIndex with an index within the container's length range",
        testRemoveByIndexWithinRange);
    suite.addTest("LinkedList removeByIndex with an index outside the container's length range",
        testRemoveByIndexOutsideRange);
    suite.addTest("LinkedList removeByIndex with removing undefined", testRemoveByIndexUndefined);
    suite.addTest("LinkedList removeByIndex with an index at the container's length range",
        testRemoveByIndexAtBoundary);
    suite.addTest("LinkedList remove the first element from an empty container", testRemoveFirstFromEmptyContainer);
    suite.addTest("LinkedList remove the first element from a non-empty container",
        testRemoveFirstFromNonEmptyContainer);
    suite.addTest("LinkedList remove the last element from an empty container", testRemoveLastFromEmptyContainer);
    suite.addTest("LinkedList remove the last element from a non-empty container", testRemoveLastFromNonEmptyContainer);
    suite.addTest("LinkedList remove an unique element", testRemoveUniqueElement);
    suite.addTest("LinkedList remove an inexistent element", testRemoveInexistentElement);
    suite.addTest("LinkedList remove a repeated element", testRemoveRepeatedElement);

    return suite.run();
}

class Temp {
    age: number = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testNumber20: number = 20;
    static readonly testNumber1w: number = 10000;
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
    static readonly forNumber2: Number[] = [1, 2, 3, 4, 5];
    static readonly arrayForNumber2 = Array.from<number>(TestData.forNumber2);
    static readonly forNumber3: Number[] = [1, 2, 4, 2, 3];
    static readonly arrayForNumber3 = Array.from<number>(TestData.forNumber3);
}

function testRemoveByIndexWithinRange() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.removeByIndex(TestData.testNumber1), TestData.testNumber1, "Removed element should be 1");
    assertEQ(linkedList.get(0), 0, "Element at index 0 should be 0");
    for (let i: number = TestData.testNumber1; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1,
        "LinkedList length should be - 1");
}

function testRemoveByIndexOutsideRange() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let errorCount = 0;
    try {
        linkedList.removeByIndex(TestData.testNumber1w);
    } catch (e) {
        if (e instanceof RangeError) {
            errorCount++;
        }
    }
    try {
        linkedList.removeByIndex(-TestData.testNumber6);
    } catch (e) {
        if (e instanceof RangeError) {
            errorCount++;
        }
    }
    assertEQ(errorCount, TestData.testNumber2, "Throw error when LinkedList index out of range");
}

function testRemoveByIndexUndefined() {
    let linkedList = new LinkedList<number | string | boolean | null | object | undefined>();
    linkedList.add(undefined);
    linkedList.add(TestData.arrayForNumber1);
    assertEQ(linkedList.removeByIndex(0), undefined, "The result should be undefined");
}

function testRemoveByIndexAtBoundary() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.removeByIndex(TestData.arrayForNumber1.length - TestData.testNumber1), TestData.testNumber20,
        "Removed element should - 1");
    assertEQ(linkedList.removeByIndex(0), 0, "Removed element should be 0");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber2,
        "LinkedList length should be - 2");
}

function testRemoveFirstFromEmptyContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.removeFirst(), 0, "Removed first element should be 0");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testRemoveFirstFromNonEmptyContainer() {
    let linkedList = new LinkedList<number>();
    try {
        linkedList.removeFirst();
    } catch (e) {
        assertTrue(e instanceof RangeError, "Throw error when LinkedList index out of range");
        return;
    }
    assertFalse(false, "Throw error when LinkedList index out of range");
}

function testRemoveLastFromEmptyContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.removeLast(), TestData.testNumber20, "Removed first element should be 20");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber1[i], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testRemoveLastFromNonEmptyContainer() {
    let linkedList = new LinkedList<number>();
    try {
        linkedList.removeLast();
    } catch (e) {
        assertTrue(e instanceof RangeError, "Throw error when LinkedList index out of range");
        return;
    }
    assertFalse(false, "Throw error when LinkedList index out of range");
}

function testRemoveUniqueElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber2);
    assertTrue(linkedList.remove(TestData.testNumber1), "Element 1 should be removed");
    assertTrue(linkedList.remove(TestData.testNumber5), "Element 5 should be removed");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber2[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber2.length - TestData.testNumber2, "LinkedList length should - 2");
}

function testRemoveInexistentElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber2);
    assertFalse(linkedList.remove(TestData.testNumber6), "Element 6 is inexistent");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber2[i], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber2.length, "LinkedList length should should not change");
}

function testRemoveRepeatedElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    assertTrue(linkedList.remove(TestData.testNumber2), "Element 2 should be removed");
    assertEQ(linkedList.get(0), TestData.testNumber1, "First element should be 1");
    for (let i: number = TestData.testNumber1; i < linkedList.length - TestData.testNumber1; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}