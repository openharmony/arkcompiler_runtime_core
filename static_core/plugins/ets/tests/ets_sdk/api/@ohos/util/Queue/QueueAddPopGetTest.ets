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

class C1 {
    name: string = "";
    age: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("Queue API tests");

    suite.addTest("Testing Queue add with number index", testQueueAddWithNumberIndex);
    suite.addTest("Testing Queue add with string index", testQueueAddWithStringIndex);
    suite.addTest("Testing Queue add with object index", testQueueAddWithObjectIndex);
    suite.addTest("Testing Queue add with mixed types (object, number, string)", testQueueAddWithMixedTypes);
    suite.addTest("Testing Queue pop operation with elements", testQueuePopWithElements);
    suite.addTest("Testing Queue pop operation without elements", testQueuePopWithoutElements);
    suite.addTest("Testing Queue getFirst operation with elements", testQueueGetFirstWithElements);
    suite.addTest("Testing Queue getFirst operation without elements", testQueueGetFirstWithoutElements);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: QUEUE_ADD_0100
function testQueueAddWithNumberIndex() {
    let queue = new Queue<number>();
    let result = queue.add(1);
    arktest.assertTrue(result, "Adding a number to the queue should return true");
}

// Test cases ported from ArkTS 1.0: QUEUE_ADD_0200
function testQueueAddWithStringIndex() {
    let queue = new Queue<string>();
    let result = queue.add("a");
    arktest.assertTrue(result, "Adding a string to the queue should return true");
}

// Test cases ported from ArkTS 1.0: QUEUE_ADD_0300
function testQueueAddWithObjectIndex() {
    let queue = new Queue<object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let result = queue.add(a);
    arktest.assertTrue(result, "Adding an object to the queue should return true");
}

// Test cases ported from ArkTS 1.0: QUEUE_ADD_0400
function testQueueAddWithMixedTypes() {
    let queue = new Queue<number | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    let result1 = queue.add(a);
    let result2 = queue.add(b);
    let result3 = queue.add(c);
    arktest.assertTrue(result1, "Adding an object to the queue should return true");
    arktest.assertTrue(result2, "Adding a string to the queue should return true");
    arktest.assertTrue(result3, "Adding a number to the queue should return true");
}

// Test cases ported from ArkTS 1.0: QUEUE_POP_0600
function testQueuePopWithElements() {
    let queue = new Queue<number | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    queue.add(a);
    queue.add(b);
    queue.add(c);
    let result = queue.pop();
    arktest.assertEQ(result, a);
}

// Test cases ported from ArkTS 1.0: QUEUE_POP_0700
function testQueuePopWithoutElements() {
    let queue = new Queue<number | string | object>();
    let result = queue.pop();
    arktest.assertEQ(result, undefined);
}

// Test cases ported from ArkTS 1.0: QUEUE_GETFIRST_0800
function testQueueGetFirstWithElements() {
    let queue = new Queue<number | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    queue.add(a);
    queue.add(b);
    queue.add(c);
    let result = queue.getFirst();
    arktest.assertEQ(result, a);
}

// Test cases ported from ArkTS 1.0: QUEUE_GETFIRST_0900
function testQueueGetFirstWithoutElements() {
    let queue = new Queue<number | string | object>();
    let result = queue.getFirst();
    arktest.assertEQ(result, undefined);
}
