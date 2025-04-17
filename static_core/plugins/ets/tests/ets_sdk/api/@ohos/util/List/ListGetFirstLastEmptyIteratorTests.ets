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

    suite.addTest("List getFirst when the container is non-empty", testGetFirstNonEmptyContainer);
    suite.addTest("List getFirst when the container is empty", testGetFirstEmptyContainer);
    suite.addTest("List getFirst when the first element is undefined", testGetFirstUndefined);
    suite.addTest("List getLast when the container is non-empty", testGetLastNonEmptyContainer);
    suite.addTest("List getLast when the container is empty", testGetLastEmptyContainer);
    suite.addTest("List getLast when the last element is undefined", testGetLastUndefined);
    suite.addTest("List isEmpty", testIsEmptyAfterAddingElement);
    suite.addTest("List isEmpty with empty container", testIsEmptyInitially);
    suite.addTest("List iterator with an empty container", testIteratorEmptyContainer);
    suite.addTest("List iterator with a non-empty container", testIteratorNonEmptyContainer);
    return suite.run();
}

function initializeList(list: List<number>, sourceArray: Array<number>) {
    for (let i = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber3: number = 3;
    static readonly testNumber20: number = 20;
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
}

function testGetFirstNonEmptyContainer() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    assertEQ(list.getFirst(), 0, "First element should be 0");
}

function testGetFirstEmptyContainer() {
    let list = new List<number>();
    assertEQ(list.getFirst(), undefined, "First element should be undefined when container is empty");
}

function testGetFirstUndefined() {
    let list = new List<number | undefined>();
    list.add(undefined);
    list.add(TestData.testNumber1);
    assertEQ(list.getFirst(), undefined, "First element should be undefined");
}

function testGetLastNonEmptyContainer() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    assertEQ(list.getLast(), TestData.testNumber20, "Last element should be 20");
}

function testGetLastEmptyContainer() {
    let list = new List<number>();
    assertEQ(list.getLast(), undefined, "Last element should be undefined when container is empty");
}

function testGetLastUndefined() {
    let list = new List<number | undefined>();
    list.add(TestData.testNumber1);
    list.add(undefined);
    assertEQ(list.getLast(), undefined, "Last element should be undefined");
}

function testIsEmptyAfterAddingElement() {
    let list = new List<number>();
    list.add(TestData.testNumber3);
    let result: boolean = list.isEmpty();
    assertFalse(result, "List should not be empty after adding an element");
}

function testIsEmptyInitially() {
    let list = new List<number>();
    let result: boolean = list.isEmpty();
    assertTrue(result, "List should be empty initially");
}

function testIteratorEmptyContainer() {
    let list = new List<number>();
    let count: number = 0;
    for (let item of list) {
        count++;
    }
    assertEQ(count, 0, "Iterator count should be 0");
}

function testIteratorNonEmptyContainer() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let count: number = 0;
    let iter = list.$_iterator();
    let temp: IteratorResult<number> = iter.next();
    while (!temp.done) {
        assertEQ(list.get(count++), temp.value, `Element should be equal`);
        temp = iter.next();
    }
    assertEQ(count, list.length, "Iterator count should match list length");
}