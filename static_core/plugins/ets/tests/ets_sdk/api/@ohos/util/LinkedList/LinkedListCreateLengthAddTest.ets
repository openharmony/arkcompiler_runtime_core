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
    const suite = new ArkTestsuite("LinkedList Create, Length, and Add API tests");

    suite.addTest("LinkedList create a single-type container", testCreateSingleTypeContainer);
    suite.addTest("LinkedList create a composite-type container", testCreateCompositeTypeContainer);
    suite.addTest("LinkedList length syncs with element count changes for single-type", testLengthSyncSingleType);
    suite.addTest("LinkedList length syncs with element count changes for multi-type", testLengthSyncMultiType);
    suite.addTest("LinkedList length with add large value", testLengthWithLargeValue);
    suite.addTest("LinkedList add elements of multiple types to the container", testAddMultipleTypes);
    suite.addTest("LinkedList add special characters to the container", testAddSpecialCharacters);
    suite.addTest("LinkedList add an element of the declared type to the first position", testAddFirstDeclaredType);
    suite.addTest("LinkedList add null to the first position", testAddFirstNull);
    suite.addTest("LinkedList add special characters to the first", testAddFirstSpecialCharacters);

    return suite.run();
}

class Temp {
    age: number = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber4: number = 4;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testNumber1w: number = 10000;
    static readonly testString1: string = "string";
    static readonly testString2: string = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<number>(TestData.forNumber1);
}

function testCreateSingleTypeContainer() {
    let linkedList: LinkedList<number>;
    try {
        linkedList = new LinkedList<number>();
    } catch (e) {
        assertFalse(false, "The memory application needs to be successful");
        return;
    }
    linkedList.add(TestData.testNumber1);
    assertTrue(linkedList instanceof LinkedList, "The instance should be LinkedList");
    assertEQ(linkedList.length, TestData.testNumber1, "LinkedList length should be 1");
}

function testCreateCompositeTypeContainer() {
    let linkedList: LinkedList<number | string | boolean | null | object | undefined>;
    try {
        linkedList = new LinkedList<number | string | boolean | null | object | undefined>();
    } catch (e) {
        assertFalse(false, "The memory application needs to be successful");
        return;
    }
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testString1);
    linkedList.add(true);
    linkedList.add(null);
    linkedList.add(TestData.testObject1);
    linkedList.add(undefined);
    assertTrue(linkedList instanceof LinkedList, "The instance should be LinkedList");
    assertEQ(linkedList.length, TestData.testNumber6, "LinkedList length should be 6");
}

function testLengthSyncSingleType() {
    let linkedList = new LinkedList<number>();
    assertEQ(linkedList.length, 0, "LinkedList length should be 0");
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    assertEQ(linkedList.length, TestData.arrayForNumber1.length,
        "LinkedList length should be TestData.arrayForNumber1 length");
    linkedList.remove(TestData.testNumber1);
    assertEQ(linkedList.length, TestData.arrayForNumber1.length - TestData.testNumber1, "LinkedList length should - 1");
}

function testLengthSyncMultiType() {
    let linkedList = new LinkedList<number | string | boolean | null | object | undefined>();
    assertEQ(linkedList.length, 0, "LinkedList length should be 0");
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testString1);
    linkedList.add(true);
    linkedList.add(null);
    linkedList.add(TestData.testObject1);
    linkedList.add(undefined);
    assertEQ(linkedList.length, TestData.testNumber6, "LinkedList length should be 6");
    linkedList.remove(TestData.testNumber1);
    assertEQ(linkedList.length, TestData.testNumber5, "LinkedList length should be 5");
}

function testLengthWithLargeValue() {
    let linkedList = new LinkedList<number>();
    for (let i: number = 0; i < TestData.testNumber1w; i++) {
        linkedList.add(TestData.testNumber1);
    }
    assertEQ(linkedList.length, TestData.testNumber1w, "LinkedList length should be 10000 after adding elements");
}

function testAddMultipleTypes() {
    let linkedList = new LinkedList<number | string | boolean | null | object | undefined>();
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testString1);
    linkedList.add(true);
    linkedList.add(null);
    linkedList.add(TestData.testObject1);
    linkedList.add(undefined);

    assertEQ(linkedList.length, TestData.testNumber6, "LinkedList length should be 6");
    assertEQ(linkedList.get(0), TestData.testNumber1, "Element at index 0 should match");
    assertEQ(linkedList.get(TestData.testNumber1), TestData.testString1, "Element at index 1 should match");
    assertEQ(linkedList.get(TestData.testNumber2), true, "Element at index 2 should match");
    assertEQ(linkedList.get(TestData.testNumber3), null, "Element at index 3 should match");
    assertEQ(linkedList.get(TestData.testNumber4), TestData.testObject1, "Element at index 4 should match");
    assertEQ(linkedList.get(TestData.testNumber5), undefined, "Element at index 5 should match");
}

function testAddSpecialCharacters() {
    let linkedList = new LinkedList<string>();
    linkedList.add(TestData.testString2);
    assertEQ(linkedList.get(0), TestData.testString2, "Add successful");
    assertEQ(linkedList.length, TestData.testNumber1, "LinkedList length should be 1");
}

function testAddFirstDeclaredType() {
    let linkedList = new LinkedList<number>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    linkedList.addFirst(TestData.testNumber4);
    assertEQ(linkedList.get(0), TestData.testNumber4, "First element should be 4");
    for (let i: number = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(linkedList.get(i + TestData.testNumber1), TestData.arrayForNumber1[i], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber1.length + TestData.testNumber1, "LinkedList length should + 1");
}

function testAddFirstNull() {
    let linkedList = new LinkedList<number | null>();
    for (let i = 0; i < TestData.arrayForNumber1.length; i++) {
        linkedList.add(TestData.arrayForNumber1[i]);
    }
    linkedList.addFirst(null);
    assertEQ(linkedList.get(0), null, "First element should be null");
    for (let i: number = 0; i < TestData.arrayForNumber1.length; i++) {
        assertEQ(linkedList.get(i + TestData.testNumber1), TestData.arrayForNumber1[i], "Element should match");
    }
    assertEQ(linkedList.length, TestData.arrayForNumber1.length + TestData.testNumber1,
        "LinkedList length should be + 1");
}

function testAddFirstSpecialCharacters() {
    let linkedList = new LinkedList<string>();
    linkedList.addFirst(TestData.testString2);
    assertEQ(linkedList.get(0), TestData.testString2, "Add successful");
    assertEQ(linkedList.length, TestData.testNumber1, "LinkedList length should be 1");
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}
