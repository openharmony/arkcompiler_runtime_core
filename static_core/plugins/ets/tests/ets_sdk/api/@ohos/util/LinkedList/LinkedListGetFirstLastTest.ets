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

    suite.addTest("LinkedList getFirst when the container is non-empty", testGetFirstNonEmptyContainer);
    suite.addTest("LinkedList getFirst when the container is empty", testGetFirstEmptyContainer);
    suite.addTest("LinkedList getFirst when the first element is undefined", testGetFirstUndefined);
    suite.addTest("LinkedList getLast when the container is non-empty", testGetLastNonEmptyContainer);
    suite.addTest("LinkedList getLast when the container is empty", testGetLastEmptyContainer);
    suite.addTest("LinkedList getLast when the last element is undefined", testGetLastUndefined);

    return suite.run();
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber20: number = 20;
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
}

function testGetFirstNonEmptyContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.getFirst(), 0, "First element should be 0");
}

function testGetFirstEmptyContainer() {
    let linkedList = new LinkedList<number>();
    try {
        linkedList.getFirst();
    } catch (e) {
        assertTrue(e instanceof RangeError, "Throw error when out of range");
        return;
    }
    assertFalse(false, "Throw error when out of range");
}

function testGetFirstUndefined() {
    let linkedList = new LinkedList<number | undefined>();
    linkedList.add(undefined);
    linkedList.add(TestData.testNumber1);
    assertEQ(linkedList.getFirst(), undefined, "First element should be undefined");
}

function testGetLastNonEmptyContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.getLast(), TestData.testNumber20, "Last element should be 20");
}

function testGetLastEmptyContainer() {
    let linkedList = new LinkedList<number>();
    try {
        linkedList.getLast();
    } catch (e) {
        assertTrue(e instanceof RangeError, "Throw error when out of range");
        return;
    }
    assertFalse(false, "Throw error when out of range");
}

function testGetLastUndefined() {
    let linkedList = new LinkedList<number | undefined>();
    linkedList.add(TestData.testNumber1);
    linkedList.add(undefined);
    assertEQ(linkedList.getLast(), undefined, "Last element should be undefined");
}