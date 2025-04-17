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

    suite.addTest("LinkedList forEach without passing any parameters", testForEachNoParams);
    suite.addTest("LinkedList forEach with a single parameter", testForEachSingleParam);
    suite.addTest("LinkedList forEach with two parameters", testForEachTwoParams);
    suite.addTest("LinkedList forEach with three parameters", testForEachThreeParams);
    suite.addTest("LinkedList forEach an empty container", testForEachEmptyContainer);
    suite.addTest("LinkedList forEach with changing the value", testForEachChangeValue);
    suite.addTest("LinkedList iterator with an empty container", testIteratorEmptyContainer);
    suite.addTest("LinkedList iterator with a non-empty container", testIteratorNonEmptyContainer);

    return suite.run();
}

class Temp {
    age: number = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber20: number = 20;
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
}

function testForEachNoParams() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: number = 0;
    linkedList.forEach(() => { count++ });
    assertEQ(count, linkedList.length, "ForEach count should match linkedList length");
}

function testForEachSingleParam() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: number = 0;
    linkedList.forEach((value: number) => {
        assertEQ(linkedList.get(count++), value, `Element should be equal`);
    });
    assertEQ(count, linkedList.length, "ForEach count should match linkedList length");
}

function testForEachTwoParams() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: number = 0;
    linkedList.forEach((value: number, index: number) => {
        assertEQ(linkedList.get(count), value, `Element should be equal`);
        assertEQ(count++, index, `The index should be equal`);
    });
    assertEQ(count, linkedList.length, "ForEach count should match linkedList length");
}

function testForEachThreeParams() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: number = 0;
    linkedList.forEach((value: number, index: number, tempLinkedList: LinkedList<number>) => {
        assertEQ(linkedList, tempLinkedList, "TempLinkedList should be this");
        assertEQ(linkedList.get(count), value, `Element should be equal`);
        assertEQ(count++, index, `The index should be equal`);
    });
    assertEQ(count, linkedList.length, "ForEach count should match linkedList length");
}

function testForEachEmptyContainer() {
    let linkedList = new LinkedList<number>();
    let count: number = 0;
    linkedList.forEach(() => { count++ });
    assertEQ(count, 0, "ForEach count should be 0");
}

function testForEachChangeValue() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    linkedList.forEach((value: number) => {
        value = TestData.testNumber1;
    });
    for (let i: number = 0; i < linkedList.length; i++) {
        assertEQ(linkedList.get(i), TestData.arrayForNumber1[i], "ForEach should keep all elements the same");
    }
}

function testIteratorEmptyContainer() {
    let linkedList = new LinkedList<number>();
    let count: number = 0;
    for (let item of linkedList) {
        count++;
    }
    assertEQ(count, 0, "Iterator count should be 0");
}

function testIteratorNonEmptyContainer() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let count: number = 0;
    let iter = linkedList.$_iterator();
    let temp: IteratorResult<number> = iter.next();
    while (!temp.done) {
        assertEQ(linkedList.get(count++), temp.value, `Element should be equal`);
        temp = iter.next();
    }
    assertEQ(count, linkedList.length, "Iterator count should match linkedList length");
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}