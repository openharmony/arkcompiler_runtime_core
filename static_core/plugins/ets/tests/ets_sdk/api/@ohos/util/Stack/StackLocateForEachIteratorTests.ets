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

import {Stack} from "@ohos.util.Stack";

function main() {
    const suite = new ArkTestsuite("Stack API tests");

    suite.addTest("Stack locate with unique element", testLocateWithUniqueElement);
    suite.addTest("Stack locate with repeated element", testLocateWithRepeatedElement);
    suite.addTest("Stack locate with inexistent element", testLocateWithInexistentElement);
    suite.addTest("Stack forEach with no parameter function", testForEachNoParameter);
    suite.addTest("Stack forEach with function with value", testForEachWithValue);
    suite.addTest("Stack forEach with function with value and index", testForEachWithValueAndIndex);
    suite.addTest("Stack forEach with function with value, index and stack", testForEachWithValueIndexAndStack);
    suite.addTest("Stack forEach with changing the elements", testForEachWithChangingElements);
    suite.addTest("Stack iterate with empty stack", testIteratorWithEmptyStack);
    suite.addTest("Stack iterate", testIterator);
    suite.addTest("Stack interface composition testing", testInterfaceComposition);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber4: number = 4;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testNumber8: number = 8;
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
}

function testLocateWithUniqueElement() {
    let stack = new Stack<number>();
    stack.push(TestData.testNumber1);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber3);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber4);
    assertEQ(stack.locate(TestData.testNumber1), 0, "Locate should return the correct index of the element");
    assertEQ(stack.locate(TestData.testNumber4), stack.length - TestData.testNumber1,
        "Locate should return the correct index of the element");
}

function testLocateWithRepeatedElement() {
    let stack = new Stack<number>();
    stack.push(TestData.testNumber1);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber3);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber4);
    assertEQ(stack.locate(TestData.testNumber2), TestData.testNumber1,
        "Locate should return the correct index of the element");

}

function testLocateWithInexistentElement() {
    let stack = new Stack<number>();
    stack.push(TestData.testNumber1);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber3);
    stack.push(TestData.testNumber2);
    stack.push(TestData.testNumber4);
    assertEQ(stack.locate(TestData.testNumber5), -TestData.testNumber1, "Locate of inexistent element should be -1");
}

function testForEachNoParameter() {
    let stack = new Stack<number>();
    initializeStack(stack, TestData.forNumber);
    let forEachCount: number = 0;
    stack.forEach(() => {
        forEachCount++;
    });
    assertEQ(forEachCount, stack.length, "ForEach count should match stack length");
}

function testForEachWithValue() {
    let stack = new Stack<number>();
    initializeStack(stack, TestData.forNumber);
    let forEachCount: int = 0;
    stack.forEach((value: number) => {
        assertEQ(value, TestData.forNumber[forEachCount], "ForEach should iterate elements in correct order");
        forEachCount++;
    });
    assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match stack length");
}

function testForEachWithValueAndIndex() {
    let stack = new Stack<number>();
    initializeStack(stack, TestData.forNumber);
    let forEachCount: int = 0;
    stack.forEach((value: number, index: number) => {
        assertEQ(value, TestData.forNumber[forEachCount], "ForEach should iterate elements in correct order");
        assertEQ(index, forEachCount, "ForEach should iterate elements in correct order");
        forEachCount++;
    });
    assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match stack length");
}

function testForEachWithValueIndexAndStack() {
    let stack = new Stack<number>();
    initializeStack(stack, TestData.forNumber);
    let forEachCount: int = 0;
    stack.forEach((value: number, index: number, tempStack: Stack<number>) => {
        assertEQ(stack, tempStack, "TempStack should be this");
        assertEQ(value, TestData.forNumber[forEachCount], "ForEach should iterate elements in correct order");
        assertEQ(index, forEachCount, "ForEach should iterate elements in correct order");
        assertEQ(stack.locate(value), tempStack.locate(value), "Locate should return the correct index of the element");

        forEachCount++;
    });
    assertEQ(forEachCount, TestData.forNumber.length, "ForEach count should match stack length");
}

function testForEachWithChangingElements() {
    let stack = new Stack<number>();
    initializeStack(stack, TestData.forNumber);

    stack.forEach((value: number) => {
        value = 0;
    });

    let forEachCount: int = 0;
    stack.forEach((value: number) => {
        assertEQ(value, TestData.forNumber[forEachCount], "ForEach should iterate elements in correct order");
        forEachCount++;
    });
}

function testIteratorWithEmptyStack() {
    let stack = new Stack<number>();
    let elem: number = 0;
    for (let item of stack) {
        elem = TestData.testNumber1;
    }
    let iter = stack.$_iterator();
    let temp: IteratorResult<number> = iter.next();
    while (!temp.done) {
        elem = TestData.testNumber1;
    }
    assertEQ(elem, 0, "Iterator over empty stack should not iterate any elements");
}

function testIterator() {
    let stack = new Stack<number>();
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        stack.push(i);
    }
    let elem1: number = TestData.testNumber1;
    for (let item of stack) {
        assertEQ(item, elem1++, "Iterator should iterate elements in correct order");
    }
    assertEQ(elem1, TestData.testNumber6, "Iterator should iterate all elements correctly");
    let iter = stack.$_iterator();
    let temp: IteratorResult<number> = iter.next();
    let elem2: number = TestData.testNumber1;
    while (!temp.done) {
        assertEQ(temp.value, elem2++, "Iterator should iterate elements in correct order");
        temp = iter.next();
    }
    assertEQ(elem2, TestData.testNumber6, "Iterator should iterate all elements correctly");
}

function testInterfaceComposition() {
    let stack = new Stack<number>();
    assertTrue(stack.isEmpty(), "Stack is empty");

    initializeStack(stack, TestData.forNumber);
    assertFalse(stack.isEmpty(), "Stack is not empty after adding elements");

    assertEQ(stack.peek(), stack.pop(), "Pop first from stack should return first element");
    assertEQ(stack.locate(TestData.testNumber2), TestData.testNumber8,
        "Locate should return the correct index of the element");

    let forEachCount: number = 0;
    stack.forEach(() => {
        forEachCount++;
    });
    assertEQ(forEachCount, stack.length, "ForEach count should match stack length");
}

function initializeStack<T>(stack: Stack<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        assertEQ(stack.push(sourceArray[i]), sourceArray[i], "The result should be the added element");
    }
}
