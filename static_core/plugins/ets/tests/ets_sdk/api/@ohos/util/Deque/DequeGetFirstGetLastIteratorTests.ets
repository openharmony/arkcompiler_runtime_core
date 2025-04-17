/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import Deque from "@ohos.util.Deque";

function main(): int {
    const suite = new ArkTestsuite("Deque API tests");

    suite.addTest("Deque get first element from an empty deque", testGetFirstFromEmptyDeque);
    suite.addTest("Deque get first element", testGetFirstFromNonEmptyDeque);
    suite.addTest("Deque get last element from an empty deque", testGetLastFromEmptyDeque);
    suite.addTest("Deque get last element", testGetLastFromNonEmptyDeque);
    suite.addTest("Deque iterate with empty deque", testIteratorWithEmptyDeque);
    suite.addTest("Deque iterate", testIteratorWithNonEmptyDeque);
    suite.addTest("Deque interface composition testing", testDequeInterfaceComposition);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber3: number = 3;
    static readonly testNumber6: number = 6;
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
}

function testGetFirstFromEmptyDeque() {
    let deque = new Deque<number>();
    assertEQ(deque.length, 0, "Initial deque length should be 0");
    assertEQ(deque.getFirst(), undefined, "Get first from empty deque should return undefined");
}

function testGetFirstFromNonEmptyDeque() {
    let deque = new Deque<number>();
    deque.insertEnd(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        deque.insertEnd(TestData.testNumber3);
    }
    assertEQ(deque.length, TestData.testNumber6, "Deque length should be 6 after inserting six elements");
    assertEQ(deque.getFirst(), TestData.testNumber1, "Get first from non-empty deque should return first element");
}

function testGetLastFromEmptyDeque() {
    let deque = new Deque<number>();
    assertEQ(deque.length, 0, "Initial deque length should be 0");
    assertEQ(deque.getLast(), undefined, "Get last from empty deque should return undefined");
}

function testGetLastFromNonEmptyDeque() {
    let deque = new Deque<number>();
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        deque.insertEnd(TestData.testNumber3);
    }
    deque.insertEnd(TestData.testNumber1);
    assertEQ(deque.length, TestData.testNumber6, "Deque length should be 6 after inserting six elements");
    assertEQ(deque.getLast(), TestData.testNumber1, "Get last from non-empty deque should return first element");
}

function testIteratorWithEmptyDeque() {
    let deque = new Deque<number>();
    let elem: number = 0;
    for (let item of deque) {
        elem = TestData.testNumber1;
    }
    let iter = deque.$_iterator();
    let temp: IteratorResult<number> = iter.next();
    while (!temp.done) {
        elem = TestData.testNumber1;
    }
    assertEQ(elem, 0, "Iterator over empty deque should not iterate any elements");
}

function testIteratorWithNonEmptyDeque() {
    let deque = new Deque<number>();
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        deque.insertEnd(i);
    }
    let elem1: number = TestData.testNumber1;
    for (let item of deque) {
        assertEQ(item, elem1++, "Iterator should iterate elements in correct order");
    }
    assertEQ(elem1, TestData.testNumber6, "Iterator should iterate all elements correctly");
    let iter = deque.$_iterator();
    let temp: IteratorResult<number> = iter.next();
    let elem2: number = TestData.testNumber1;
    while (!temp.done) {
        assertEQ(temp.value, elem2++, "Iterator should iterate elements in correct order");
        temp = iter.next();
    }
    assertEQ(elem2, TestData.testNumber6, "Iterator should iterate all elements correctly");
}

function testDequeInterfaceComposition() {
    let deque = new Deque<number>();
    deque.insertFront(TestData.testNumber3);
    assertEQ(deque.length, TestData.testNumber1, "Deque length should be 1 after inserting one element at the front");

    initializeDeque(deque, TestData.forNumber);
    let result: boolean = deque.has(TestData.testNumber3);
    assertTrue(result, "Deque should contain the element");

    assertEQ(deque.getFirst(), deque.popFirst(), "Pop first from deque should return first element");
    assertEQ(deque.getLast(), deque.popLast(), "Pop last from deque should return last element");

    let forEachCount: number = 0;
    deque.forEach(() => {
        forEachCount++;
    });
    assertEQ(forEachCount, deque.length, "ForEach count should match deque length");
}

function initializeDeque<T>(deque: Deque<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        deque.insertEnd(sourceArray[i]);
    }
}
