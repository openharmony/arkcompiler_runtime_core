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

    suite.addTest("LinkedList removeFirstFound with an unique element", testRemoveFirstFoundUniqueElement);
    suite.addTest("LinkedList removeFirstFound with an inexistent element", testRemoveFirstFoundInexistentElement);
    suite.addTest("LinkedList removeFirstFound with a repeated element", testRemoveFirstFoundRepeatedElement);
    suite.addTest("LinkedList removeLastFound with an unique element", testRemoveLastFoundUniqueElement);
    suite.addTest("LinkedList removeLastFound with an inexistent element", testRemoveLastFoundInexistentElement);
    suite.addTest("LinkedList removeLastFound add->getLastIndexOf->removeLastFound",
        testRemoveLastFoundAddGetLastIndexOf);
    suite.addTest("LinkedList removeLastFound with a repeated element", testRemoveLastFoundRepeatedElement);

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
    static readonly testNumber6: number = 6;
    static readonly forNumber3: Number[] = [1, 2, 4, 2, 3];
    static readonly arrayForNumber3 = Array.from<number>(TestData.forNumber3);
}

function testRemoveFirstFoundUniqueElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    assertTrue(linkedList.removeFirstFound(TestData.testNumber1), "First found element 1 should be removed");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testRemoveFirstFoundInexistentElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    assertFalse(linkedList.removeFirstFound(TestData.testNumber6), "Element 6 is inexistent");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber3[i], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber3.length, "LinkedList length should should not change");
}

function testRemoveFirstFoundRepeatedElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    assertTrue(linkedList.removeFirstFound(TestData.testNumber2), "First found element 2 should be removed");
    assertEQ(linkedList.get(0), TestData.testNumber1, "First element should be 1");
    for (let i: number = TestData.testNumber1; i < linkedList.length - TestData.testNumber1; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testRemoveLastFoundUniqueElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    assertTrue(linkedList.removeLastFound(TestData.testNumber1), "Last found element 1 should be removed");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testRemoveLastFoundInexistentElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    assertFalse(linkedList.removeLastFound(TestData.testNumber6), "Element 6 is inexistent");
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber3[i], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber3.length, "LinkedList length should should not change");
}

function testRemoveLastFoundAddGetLastIndexOf() {
    let linkedList = new LinkedList<number>();
    linkedList.add(TestData.testNumber2);
    let index1 = linkedList.getLastIndexOf(TestData.testNumber2);
    linkedList.add(TestData.testNumber2);
    let index2 = linkedList.getLastIndexOf(TestData.testNumber2);
    linkedList.add(TestData.testNumber2);
    let index3 = linkedList.getLastIndexOf(TestData.testNumber2);
    linkedList.add(TestData.testNumber3);
    assertTrue(index1 !== index2 && index2 !== index3, "The index is not the same");
    linkedList.removeLastFound(TestData.testNumber2);
    assertEQ(linkedList.getLastIndexOf(TestData.testNumber2), index2, "The index should be index2");
}

function testRemoveLastFoundRepeatedElement() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber3);
    assertTrue(linkedList.removeLastFound(TestData.testNumber2), "Last found element 2 should be removed");
    for (let i: number = 0; i < linkedList.length - TestData.testNumber1; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber3[i], "Element should match");
    }
    assertEQ(linkedList.get(TestData.testNumber3), TestData.testNumber3, "Last element should be 3");
    assertEQ(linkedList.length, TestData.arrayForNumber3.length - TestData.testNumber1, "LinkedList length should - 1");
}