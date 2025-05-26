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

    suite.addTest("List remove an unique element", testRemoveUniqueElement);
    suite.addTest("List remove an inexistent element", testRemoveInexistentElement);
    suite.addTest("List remove a repeated element", testRemoveRepeatedElement);
    suite.addTest("List replaceAllElements with constant value", testReplaceAllElementsWithConstantValue);
    suite.addTest("List replaceAllElements with lambda", testReplaceAllElementsWithLambda);
    suite.addTest("List replaceAllElements with lambda and index", testReplaceAllElementsWithLambdaAndIndex);
    suite.addTest("List replaceAllElements with lambda, index and tempList",
        testReplaceAllElementsWithLambdaIndexAndTempList);
    suite.addTest("List forEach without passing any parameters", testForEachNoParams);
    suite.addTest("List forEach with a single parameter", testForEachSingleParam);
    suite.addTest("List forEach with two parameters", testForEachTwoParams);
    suite.addTest("List forEach with three parameters", testForEachThreeParams);
    suite.addTest("List forEach an empty container", testForEachEmptyContainer);
    suite.addTest("List forEach with changing the value", testForEachChangeValue);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
    static readonly forNumber2: Number[] = [1, 2, 3, 4, 5];
    static readonly arrayForNumber2 = Array.from<number>(TestData.forNumber2);
    static readonly forNumber3: Number[] = [1, 2, 4, 2, 3];
    static readonly arrayForNumber3 = Array.from<number>(TestData.forNumber3);
}

function testRemoveUniqueElement() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber2);
    assertTrue(list.remove(TestData.testNumber1), "Element 1 should be removed");
    assertTrue(list.remove(TestData.testNumber5), "Element 5 should be removed");
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber2[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(list.length, TestData.arrayForNumber2.length - TestData.testNumber2, "List length should - 2");
}

function testRemoveInexistentElement() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber2);
    assertFalse(list.remove(TestData.testNumber6), "Element 6 is inexistent");
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber2[i], "Element should match");
    }
    assertEQ(list.length, TestData.arrayForNumber2.length, "List length should should not change");
}

function testRemoveRepeatedElement() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber3);
    assertTrue(list.remove(TestData.testNumber2), "Element 2 should be removed");
    assertEQ(list.get(0), TestData.testNumber1, "First element should be 1");
    for (let i: number = TestData.testNumber1; i < list.length - TestData.testNumber1; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber3[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(list.length, TestData.arrayForNumber3.length - TestData.testNumber1, "List length should - 1");
}

function testReplaceAllElementsWithConstantValue() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    list.replaceAllElements((): number => {
        return TestData.testNumber3;
    });
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.testNumber3, `Element at index ${i} should match`);
    }
}

function testReplaceAllElementsWithLambda() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    list.replaceAllElements((value: number): number => {
        return value + TestData.testNumber3;
    });
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber1[i] + TestData.testNumber3, `Element at index ${i} should match`);
    }
}

function testReplaceAllElementsWithLambdaAndIndex() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    list.replaceAllElements((value: number, index: number): number => {
        return value + index;
    });
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber1[i] + i, `Element at index ${i} should match`);
    }
}

function testReplaceAllElementsWithLambdaIndexAndTempList() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    list.replaceAllElements((value: number, index: number, tempList: List<number>): number => {
        assertEQ(list, tempList, "TempList should be this");
        return value + tempList.get(index)!;
    });
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber1[i] * 2, `Element at index ${i} should match`);
    }
}

function testForEachNoParams() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let count: number = 0;
    list.forEach(() => { count++ });
    assertEQ(count, list.length, "ForEach count should match list length");
}

function testForEachSingleParam() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let count: number = 0;
    list.forEach((value: number) => {
        assertEQ(list.get(count++), value, `Element should be equal`);
    });
    assertEQ(count, list.length, "ForEach count should match list length");
}

function testForEachTwoParams() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let count: number = 0;
    list.forEach((value: number, index: number) => {
        assertEQ(list.get(count), value, `Element should be equal`);
        assertEQ(count++, index, `The index should be equal`);
    });
    assertEQ(count, list.length, "ForEach count should match list length");
}

function testForEachThreeParams() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let count: number = 0;
    list.forEach((value: number, index: number, tempList: List<number>) => {
        assertEQ(list, tempList, "TempList should be this");
        assertEQ(list.get(count), value, `Element should be equal`);
        assertEQ(count++, index, `The index should be equal`);
    });
    assertEQ(count, list.length, "ForEach count should match list length");
}

function testForEachEmptyContainer() {
    let list = new List<number>();
    let count: number = 0;
    list.forEach(() => { count++ });
    assertEQ(count, 0, "ForEach count should be 0");
}

function testForEachChangeValue() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    list.forEach((value: number) => {
        value = TestData.testNumber1;
    });
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber1[i], "ForEach should keep all elements the same");
    }
}

function initializeList(list: List<number>, sourceArray: Array<number>) {
    for (let i = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}