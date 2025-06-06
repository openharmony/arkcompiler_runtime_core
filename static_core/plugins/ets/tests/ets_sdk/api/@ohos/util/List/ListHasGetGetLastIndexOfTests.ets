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

    suite.addTest("List has an element in the container", testHasElementInContainer);
    suite.addTest("List has an element not in the container", testHasElementNotInContainer);
    suite.addTest("List has with every type", testHasEveryType);
    suite.addTest("List get with an index within the container's length range", testGetWithinRange);
    suite.addTest("List get with an index outside the container's length range", testGetOutsideRange);
    suite.addTest("List getLastIndexOf an unique element", testGetLastIndexOfUniqueElement);
    suite.addTest("List getLastIndexOf an inexistent element", testGetLastIndexOfInexistentElement);
    suite.addTest("List getLastIndexOf add->getLastIndexOf->removeByIndex", testGetLastIndexOfAddRemoveByIndex);
    suite.addTest("List getLastIndexOf a repeated element", testGetLastIndexOfRepeatedElement);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber4: number = 4;
    static readonly testNumber20: number = 20;
    static readonly testNumber1w: number = 10000;
    static readonly testString1: string = "string";
    static readonly testString2: string = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    static readonly testObject2 = [0, 1, null];
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
    static readonly forNumber4: Number[] = [1, 2, 2, 3, 3];
    static readonly arrayForNumber4 = Array.from<number>(TestData.forNumber4);
}

function testHasElementInContainer() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    assertTrue(list.has(TestData.testNumber1), "List should contain value 1");
}

function testHasElementNotInContainer() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    let i = 0;
    list.forEach((value: number) => {
        assertEQ(value, TestData.arrayForNumber1[i++], "The element should be present");
    });
    assertFalse(list.has(TestData.testNumber1w), "List should not contain value 10000");
}

function testHasEveryType() {
    let list = new List<number | string | boolean | null | object | undefined>();
    list.add(TestData.testString2);
    assertEQ(list.getLast(), TestData.testString2, "The element should be present");
    list.add(TestData.testString1);
    assertEQ(list.getLast(), TestData.testString1, "The element should be present");
    list.add(TestData.testNumber1);
    assertEQ(list.getLast(), TestData.testNumber1, "The element should be present");
    list.add(null);
    assertEQ(list.getLast(), null, "The element should be present");
    list.add(true);
    assertEQ(list.getLast(), true, "The element should be present");
    list.add(TestData.testObject2);
    assertEQ(list.getLast(), TestData.testObject2, "The element should be present");

    assertTrue(list.has(TestData.testString2), "The element should be present");
    assertTrue(list.has(TestData.testString1), "The element should be present");
    assertTrue(list.has(true), "The element should be present");
    assertTrue(list.has(null), "The element should be present");
    assertTrue(list.has(TestData.testNumber1), "The element should be present");
    assertTrue(list.has(TestData.testObject2), "The element should be present");
}

function testGetWithinRange() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    assertEQ(list.get(0), 0, "Element at index 0 should be 0");
    assertEQ(list.get(TestData.testNumber1), TestData.testNumber1, "Element at index 1 should be 1");
    assertEQ(list.get(TestData.arrayForNumber1.length - TestData.testNumber1), TestData.testNumber20,
        "Element at index 20 should be 20");
}

function testGetOutsideRange() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber1);
    assertEQ(list.get(TestData.testNumber1w), undefined, "Element at index 10000 should be undefined");
}

function testGetLastIndexOfUniqueElement() {
    let list = new List<number>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber3);
    assertEQ(list.getLastIndexOf(TestData.testNumber1), 0, "Last index of 1 should be 0");
    assertEQ(list.getLastIndexOf(TestData.testNumber3), TestData.testNumber3, "Last index of 3 should be 3");
}

function testGetLastIndexOfInexistentElement() {
    let list = new List<number>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber3);
    assertEQ(list.getLastIndexOf(TestData.testNumber4), -TestData.testNumber1, "Last index of 4 should be -1");
}

function testGetLastIndexOfAddRemoveByIndex() {
    let list = new List<number>();
    initializeList(list, TestData.arrayForNumber4);
    let result1 = list.removeByIndex(list.getLastIndexOf(TestData.testNumber2));
    let result2 = list.removeByIndex(list.getLastIndexOf(TestData.testNumber2));
    assertEQ(result1, result2, "The two result should be equal");
    let result3 = list.removeByIndex(list.getLastIndexOf(TestData.testNumber3));
    let result4 = list.removeByIndex(list.getLastIndexOf(TestData.testNumber3));
    assertEQ(result3, result4, "The two result should be equal");
}

function testGetLastIndexOfRepeatedElement() {
    let list = new List<number>();
    list.add(TestData.testNumber1);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber2);
    list.add(TestData.testNumber3);
    assertEQ(list.getLastIndexOf(TestData.testNumber2), TestData.testNumber2, "Last index of 2 should be 2");
}

function initializeList(list: List<number>, sourceArray: Array<number>) {
    for (let i = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}