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

import {Queue} from "@ohos.util.Queue";

function main() {
    const suite = new ArkTestsuite("Queue API tests");

    suite.addTest("Queue create an instance that stores single-type data", testQueueCreateSingleType);
    suite.addTest("Queue create an instance that stores multiple types of data", testQueueCreateMultipleTypes);
    suite.addTest("Queue length", testQueueLengthBasic);
    suite.addTest("Queue length with mixed types", testQueueLengthMixedTypes);
    suite.addTest("Queue length with large number of elements", testQueueLengthLargeNumber);
    suite.addTest("Queue add", testQueueAddElements);
    suite.addTest("Queue pop first element from an empty queue", testQueuePopEmpty);
    suite.addTest("Queue pop first element", testQueuePopFirstElement);

    return suite.run();
}

class Temp {
    name: string = "";
    age: number = 0;
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testNumber1w: number = 10000;
    static readonly testObject1: Temp = { name: "Dylon", age: 13 };
    static readonly testString1: string = "string";
    static readonly forNumber: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];
}

function testQueueCreateSingleType() {
    let queue: Queue<number>;
    try {
        queue = new Queue<number>();
    } catch (e) {
        assertTrue(false, "Failed to create a single-type instance of Queue");
        return;
    }
    queue.add(TestData.testNumber3);
    assertEQ(queue.length, TestData.testNumber1, "Queue length should be 1 after adding one element");
}

function testQueueCreateMultipleTypes() {
    let queue: Queue<number | string | object>;
    try {
        queue = new Queue<number | string | object>();
    } catch (e) {
        assertTrue(false, "Failed to create a single-type instance of Queue");
        return;
    }
    queue.add(TestData.testString1);
    queue.add(TestData.testObject1);
    queue.add(TestData.testNumber3);
    queue.add(TestData.testString1);
    queue.add(TestData.testObject1);
    assertEQ(queue.length, TestData.testNumber5, "Queue length should be 5 after adding five elements");
}

function testQueueLengthBasic() {
    let queue = new Queue<number>();
    assertEQ(queue.length, 0, "Initial queue length should be 0");
    queue.add(TestData.testNumber3);
    queue.add(TestData.testNumber1);
    queue.add(TestData.testNumber2);
    assertEQ(queue.length, TestData.testNumber3, "Queue length should be 3 after adding three elements");
    queue.pop();
    queue.pop();
    assertEQ(queue.length, TestData.testNumber1, "Queue length should be 1 after popping two elements");
}

function testQueueLengthMixedTypes() {
    let queue = new Queue<number | string | boolean>();
    assertEQ(queue.length, 0, "Initial queue length should be 0");
    queue.add(TestData.testNumber3);
    queue.add(TestData.testString1);
    queue.add(true);
    assertEQ(queue.length, TestData.testNumber3, "Queue length should be 3 after adding three elements of mixed types");
    queue.pop();
    queue.pop();
    assertEQ(queue.length, TestData.testNumber1, "Queue length should be 1 after popping two elements");
}

function testQueueLengthLargeNumber() {
    let queue = new Queue<number>();
    for (let i: number = 0; i < TestData.testNumber1w; i++) {
        queue.add(TestData.testNumber1);
    }
    assertEQ(queue.length, TestData.testNumber1w, "Queue length should be 10000 after adding elements");
}

function testQueueAddElements() {
    let queue = new Queue<number>();
    initializeQueue(queue, TestData.forNumber);
    let countForEach = 0;
    queue.forEach((value: number) => {
        assertEQ(value, TestData.forNumber[countForEach++], "Element should match");
    });
    assertEQ(countForEach, queue.length, "Queue length same as foreach");
    assertEQ(queue.length, TestData.forNumber.length, "Queue length same as TestData.forNumber");
}

function testQueuePopEmpty() {
    let queue = new Queue<number>();
    assertEQ(queue.length, 0, "Initial queue length should be 0");
    assertEQ(queue.pop(), undefined, "Pop from empty queue should return undefined");
}

function testQueuePopFirstElement() {
    let queue = new Queue<number>();
    queue.add(TestData.testNumber1);
    for (let i: number = TestData.testNumber1; i < TestData.testNumber6; i++) {
        queue.add(TestData.testNumber3);
    }
    assertEQ(queue.length, TestData.testNumber6, "Queue length should be 6 after inserting six elements");
    assertEQ(queue.pop(), TestData.testNumber1, "Pop from non-empty queue should return first element");
    assertEQ(queue.length, TestData.testNumber5, "Pop from non-empty queue should update length");
    let countForEach = 0;
    queue.forEach((value: number) => {
        assertEQ(value, TestData.testNumber3, "Element should match");
        countForEach++;
    });
    assertEQ(countForEach, queue.length, "Queue length same as foreach");
}

function initializeQueue<T>(queue: Queue<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        assertTrue(queue.add(sourceArray[i]), `Failed to add element at index ${i}`);
    }
}
