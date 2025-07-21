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
import {BusinessError} from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

function main() {
    const suite = new arktest.ArkTestsuite("LinkedList API tests");

    suite.addTest("LinkedList insert an element within the container's length range", testInsertWithinRange);
    suite.addTest("LinkedList insert an element at the boundary of the container", testInsertAtBoundary);
    suite.addTest("LinkedList insert insert->addFirst->getIndexOf->removeByIndex->has",
        testInsertAddFirstGetIndexOfRemoveByIndexHas);
    suite.addTest("LinkedList insert elements at index > length", testInsertIndexGreaterThanLength);
    suite.addTest("LinkedList insert elements at index < 0", testInsertIndexLessThanZero);
    suite.addTest("LinkedList has an element in the container", testHasElementInContainer);
    suite.addTest("LinkedList has an element not in the container", testHasElementNotInContainer);
    suite.addTest("LinkedList has with every type", testHasEveryType);
    suite.addTest("LinkedList get with an index within the container's length range", testGetWithinRange);
    suite.addTest("LinkedList get with an index outside the container's length range", testGetOutsideRange);

    return suite.run();
}

class Temp {
    age: double = 0;
    name: string = "";
}

class TestData {
    static readonly testNumber1: double | int = 1;
    static readonly testNumber2: double | int = 2;
    static readonly testNumber3: double | int = 3;
    static readonly testNumber4: double | int = 4;
    static readonly testNumber5: double | int = 5;
    static readonly testNumber6: double | int = 6;
    static readonly testNumber7: double | int = 7;
    static readonly testNumber8: double | int = 8;
    static readonly testNumber20: double | int = 20;
    static readonly testNumber1w: double | int = 10000;
    static readonly testString1: string = "string";
    static readonly testString2: string = "!\"#$%'()+,-./:;<=>?@[\\]^_`{|}~";
    static readonly testObject1: Temp = { age: 10, name: "zhang" };
    static readonly testObject2 = [0, 1, null];
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly arrayForNumber1 = Array.from<double>(TestData.forNumber1);
}

function testInsertWithinRange() {
    let linkedList = new LinkedList<double | string | boolean | null | object | undefined>();
    linkedList.add(TestData.testNumber1);
    linkedList.add(TestData.testNumber2);
    linkedList.insert(TestData.testNumber1, TestData.testNumber1);
    linkedList.insert(TestData.testNumber2, TestData.testString1);
    linkedList.insert(TestData.testNumber3, true);
    linkedList.insert(TestData.testNumber4, null);
    linkedList.insert(TestData.testNumber5, TestData.testObject1);
    linkedList.insert(TestData.testNumber6, undefined);

    arktest.assertEQ(linkedList.length, TestData.testNumber8, "LinkedList length should be 8");
    arktest.assertEQ(linkedList.get(0), TestData.testNumber1, "Element at index 0 should match");
    arktest.assertEQ(linkedList.get(TestData.testNumber1), TestData.testNumber1, "Element at index 6 should match");
    arktest.assertEQ(linkedList.get(TestData.testNumber2), TestData.testString1, "Element at index 1 should match");
    arktest.assertEQ(linkedList.get(TestData.testNumber3), true, "Element at index 2 should match");
    arktest.assertEQ(linkedList.get(TestData.testNumber4), null, "Element at index 3 should match");
    arktest.assertEQ(linkedList.get(TestData.testNumber5), TestData.testObject1, "Element at index 4 should match");
    arktest.assertEQ(linkedList.get(TestData.testNumber6), undefined, "Element at index 5 should match");
    arktest.assertEQ(linkedList.get(TestData.testNumber7), TestData.testNumber2, "Element at index 7 should match");
}

function testInsertAtBoundary() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    linkedList.insert(TestData.arrayForNumber1.length.toInt(), 0);
    linkedList.insert(0, 0);

    for (let i: int = 0; i < TestData.arrayForNumber1.length.toInt(); i++) {
        arktest.assertEQ(linkedList.get(i + TestData.testNumber1), TestData.arrayForNumber1[i], "Element should match");
    }
    arktest.assertEQ(linkedList.get(0), 0, "Element should be 0");
    arktest.assertEQ(linkedList.get(linkedList.length - TestData.testNumber1), 0, "Element should be 0");
    arktest.assertEQ(linkedList.length, TestData.arrayForNumber1.length.toInt() + TestData.testNumber2, "LinkedList length should + 2");
}

function testInsertAddFirstGetIndexOfRemoveByIndexHas() {
    let linkedList = new LinkedList<double | string>();
    linkedList.insert(0, TestData.testNumber1);
    arktest.assertEQ(linkedList.get(0), TestData.testNumber1, "Element should be 0");

    linkedList.addFirst(TestData.testString2);
    let index = linkedList.getIndexOf(TestData.testString2);
    arktest.assertEQ(index, 0, "The index of the first element should be 0");

    linkedList.removeByIndex(index);
    arktest.assertFalse(linkedList.has(TestData.testString2), "The element should be removed");
}

function testInsertIndexGreaterThanLength() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError && (e as BusinessError).name == 'BusinessError') && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= 21. Received value is: 10000") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {linkedList.insert(TestData.testNumber1w, TestData.testNumber6)}, exceptionCheck);
}

function testInsertIndexLessThanZero() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError && (e as BusinessError).name == 'BusinessError') && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= 21. Received value is: -6") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {linkedList.insert(-TestData.testNumber6, TestData.testNumber1)}, exceptionCheck);
}

function testHasElementInContainer() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertTrue(linkedList.has(TestData.testNumber1), "LinkedList should contain value 1");
}

function testHasElementNotInContainer() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertFalse(linkedList.has(TestData.testNumber1w), "LinkedList should not contain value 10000");
}

function testHasEveryType() {
    let linkedList = new LinkedList<double | string | boolean | null | object | undefined>();

    linkedList.add(TestData.testString2);
    arktest.assertEQ(linkedList.getLast(), TestData.testString2, "The element should be present");

    linkedList.add(TestData.testString1);
    arktest.assertEQ(linkedList.getLast(), TestData.testString1, "The element should be present");

    linkedList.add(TestData.testNumber1);
    arktest.assertEQ(linkedList.getLast(), TestData.testNumber1, "The element should be present");

    linkedList.add(null);
    arktest.assertEQ(linkedList.getLast(), null, "The element should be present");

    linkedList.add(true);
    arktest.assertEQ(linkedList.getLast(), true, "The element should be present");

    linkedList.add(TestData.testObject2);
    arktest.assertEQ(linkedList.getLast(), TestData.testObject2, "The element should be present");
}

function testGetWithinRange() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertEQ(linkedList.get(0), 0, "Element at index 0 should be 0");
    arktest.assertEQ(linkedList.get(TestData.testNumber1), TestData.testNumber1, "Element at index 1 should be 1");
    arktest.assertEQ(linkedList.get(TestData.arrayForNumber1.length.toInt() - TestData.testNumber1), TestData.testNumber20,
        "Element at index 20 should be 20");
}

function testGetOutsideRange() {
    let linkedList = new LinkedList<double>();
    initializeLinkedList(linkedList, TestData.arrayForNumber1);
    arktest.assertEQ(linkedList.get(TestData.testNumber1w), undefined, "Element at index 10000 should be undefined");
}

function initializeLinkedList<T>(linkedList: LinkedList<T>, sourceArray: Array<T>) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        linkedList.add(sourceArray[i]);
    }
}
