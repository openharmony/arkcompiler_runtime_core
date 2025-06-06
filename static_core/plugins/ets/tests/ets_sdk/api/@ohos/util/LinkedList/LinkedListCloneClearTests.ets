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

    suite.addTest("LinkedList clone a non-empty container", testCloneNonEmptyContainer);
    suite.addTest("LinkedList clone an empty container", testCloneEmptyContainer);
    suite.addTest("LinkedList clone and delete elements from the original LinkedList", testCloneAndDeleteFromOriginal);
    suite.addTest("LinkedList clone and add elements to the newLinkedList", testCloneAndAddToNew);
    suite.addTest("LinkedList clear a non-empty container", testClearNonEmptyContainer);
    suite.addTest("LinkedList clear an empty container", testClearEmptyContainer);
    suite.addTest("LinkedList clear add->clone->clear->add", testClearAddCloneClearAdd1);
    suite.addTest("LinkedList clear add->clone->clear->add", testClearAddCloneClearAdd2);

    return suite.run();
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
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
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
}

function testCloneNonEmptyContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let newLinkedList = linkedList.clone();
    for (let i = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), newLinkedList.get(i), `Element at index ${i} should be equal`);
    }
    assertEQ(newLinkedList.length, linkedList.length, "Cloned LinkedList length should be same as origin LinkedList");
}

function testCloneEmptyContainer() {
    let linkedList = new LinkedList<number>();
    let newLinkedList: LinkedList<number> = linkedList.clone();
    assertEQ(newLinkedList.length, 0, "Cloned LinkedList length should be 0");
}

function testCloneAndDeleteFromOriginal() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let newLinkedList = linkedList.clone();
    linkedList.removeFirst();
    assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1,
        "Origin LinkedList length should - 1");
    for (let i = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(TestData.arrayForNumber1[i], newLinkedList.get(i), `Element at index ${i} should be equal`);
    }
    assertEQ(newLinkedList.length, TestData.arrayForNumber1.length, "Cloned LinkedList length should not change");
}

function testCloneAndAddToNew() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let newLinkedList = linkedList.clone();
    newLinkedList.add(TestData.testNumber1);
    assertEQ(newLinkedList.length, TestData.arrayForNumber1.length + TestData.testNumber1,
        "Cloned LinkedList length should + 1");
    for (let i = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(TestData.arrayForNumber1[i], linkedList.get(i), `Element at index ${i} should be equal`);
    }
    assertEQ(linkedList.length, TestData.arrayForNumber1.length, "Origin LinkedList length should not change");
}

function testClearNonEmptyContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.length, TestData.arrayForNumber1.length,
        "LinkedList length should be same as TestData.arrayForNumber1");
    linkedList.clear();
    assertEQ(linkedList.length, 0, "LinkedList length should be 0 after clear");
}

function testClearEmptyContainer() {
    let linkedList = new LinkedList<number>();
    assertEQ(linkedList.length, 0, "LinkedList length should be 0");
    linkedList.clear();
    assertEQ(linkedList.length, 0, "LinkedList length should be 0 after clear");
}

function testClearAddCloneClearAdd1() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let newLinkedList = linkedList.clone();
    newLinkedList.clear();
    newLinkedList.add(TestData.testNumber1);
    assertEQ(newLinkedList.length, TestData.testNumber1, "Cloned LinkedList length should be 1");
    assertEQ(linkedList.length, TestData.arrayForNumber1.length, "Origin LinkedList length should not change");
}

function testClearAddCloneClearAdd2() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let newLinkedList = linkedList.clone();
    linkedList.clear();
    linkedList.add(TestData.testNumber1);
    assertEQ(linkedList.length, TestData.testNumber1, "Origin LinkedList length should be 1");
    assertEQ(newLinkedList.length, TestData.arrayForNumber1.length, "Cloned LinkedList length should not change");
}