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

    suite.addTest("List getIndexOf an unique element", testGetIndexOfUniqueElement);
    suite.addTest("List getIndexOf an inexistent element", testGetIndexOfInexistentElement);
    suite.addTest("List getIndexOf add->getIndexOf->removeByIndex", testGetIndexOfAddRemoveByIndex);
    suite.addTest("List getIndexOf a repeated element", testGetIndexOfRepeatedElement);
    suite.addTest("List equal two containers with same elements", testEqualSameElement);
    suite.addTest("List equal two containers with different elements", testEqualDifferentElement);
    suite.addTest("List equal same container", testEqualSameContainer);
    suite.addTest("List removeByIndex with an index within the container's length range", testRemoveByIndexWithinRange);
    suite.addTest("List removeByIndex with an index outside the container's length range",
        testRemoveByIndexOutsideRange);
    suite.addTest("List removeByIndex with removing undefined", testRemoveByIndexUndefined);
    suite.addTest("List removeByIndex with an index at the container's length range", testRemoveByIndexAtBoundary);

    return suite.run();
}

function initializeList(list: List<number>, sourceArray: Array<number>) {
    for (let i = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber4: number = 4;
    static readonly testNumber6: number = 6;
    static readonly testNumber20: number = 20;
    static readonly testNumber1w: number = 10000;
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
    static readonly forNumber2: Number[] = [1, 2, 3, 4, 5];
    static readonly arrayForNumber2 = Array.from<number>(TestData.forNumber2);
    static readonly forNumber4: Number[] = [1, 2, 2, 3, 3];
    static readonly arrayForNumber4 = Array.from<number>(TestData.forNumber4);
}

function testGetIndexOfUniqueElement() {
    let list = new List<number>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber3);
    assertEQ(list.getIndexOf(TestData.testNumber1), 0, "First index of 1 should be 0");
    assertEQ(list.getIndexOf(TestData.testNumber3), TestData.testNumber3, "First index of 3 should be 3");
}

function testGetIndexOfInexistentElement() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber4);
    assertEQ(list.getIndexOf(TestData.testNumber4), -TestData.testNumber1, "First index of 4 should be -1");
}

function testGetIndexOfAddRemoveByIndex() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber4);
    let result1 = list.removeByIndex(list.getIndexOf(TestData.testNumber2));
    let result2 = list.removeByIndex(list.getIndexOf(TestData.testNumber2));
    assertEQ(result1, result2, "The two result should be equal");
    let result3 = list.removeByIndex(list.getIndexOf(TestData.testNumber3));
    let result4 = list.removeByIndex(list.getIndexOf(TestData.testNumber3));
    assertEQ(result3, result4, "The two result should be equal");
}

function testGetIndexOfRepeatedElement() {
    let list = new List<number>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber3);
    assertEQ(list.getIndexOf(TestData.testNumber2), TestData.testNumber1, "First index of 2 should be 1");
}

function testEqualSameElement(){
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber2);
    let list1= new List<number>();
    initializeList(list1, TestData.arrayForNumber2);
    assertTrue(list.equal(list1), "Lists should be equal");
}

function testEqualDifferentElement(){
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber2);
    let list1 = new List<number>();
    initializeList(list1, TestData.arrayForNumber4);
    assertFalse(list.equal(list1), "Lists should not be equal");
}

function testEqualSameContainer(){
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber2);
    assertTrue(list.equal(list), "List should be equal to itself");
}

function testRemoveByIndexWithinRange() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    assertEQ(list.removeByIndex(TestData.testNumber1), TestData.testNumber1, "Removed element should be 1");
    assertEQ(list.get(0), 0, "Element at index 0 should be 0");
    for (let i: number = TestData.testNumber1; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(list.length, TestData.arrayForNumber1.length - TestData.testNumber1, "List length should be - 1");
}

function testRemoveByIndexOutsideRange() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let errorCount = 0;
    try {
        list.removeByIndex(TestData.testNumber1w);
    } catch (e) {
        if (e instanceof RangeError) {
            assertEQ((e as RangeError).message, "The value of index is out of range.",
                "The result should be 'Index out of range'");
            errorCount++;
        }
    }
    try {
        list.removeByIndex(-TestData.testNumber6);
    } catch (e) {
        if (e instanceof RangeError) {
            assertEQ((e as RangeError).message, "The value of index is out of range.",
                "The result should be 'Index out of range'");
            errorCount++;
        }
    }
    assertEQ(errorCount, TestData.testNumber2, "The result should be 'RangeError'");
}

function testRemoveByIndexUndefined() {
    let list = new List<number | string | boolean | null | object | undefined>();
    list.add(undefined);
    list.add(TestData.arrayForNumber1);
    assertEQ(list.removeByIndex(0), undefined, "The result should be undefined");
}

function testRemoveByIndexAtBoundary() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    assertEQ(list.removeByIndex(TestData.arrayForNumber1.length - TestData.testNumber1), TestData.testNumber20,
        "Removed element should - 1");
    assertEQ(list.removeByIndex(0), 0, "Removed element should be 0");
    for (let i: number = 0; i < list.length; i++) {
        assertEQ(list.get(i), TestData.arrayForNumber1[i + TestData.testNumber1], "Element should match");
    }
    assertEQ(list.length, TestData.arrayForNumber1.length - TestData.testNumber2, "List length should be - 2");
}