/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import Deque from "@ohos.util.Deque";
import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: number = 401;

// Test cases ported from ArkTS 1.0: container_deque.js
function main() {
    const suite = new arktest.ArkTestsuite("Deque API tests");

    suite.addTest("Testing basic operations of Deque", testBasicOperations);
    suite.addTest("Testing exception handling for invalid index in Deque", testExceptionHandlingInvalidIndex);
    suite.addTest("Testing popFirst and iteration in Deque", testPopFirstAndIteration);
    suite.addTest("Testing key iteration in Deque", testKeyIteration);

    return suite.run();
}

function testBasicOperations() {
    let deque = new Deque<number>();
    let testArray = new Array<number>();
    for (let i = 0; i < 10; i++) {
        testArray.push(i);
    }
    for (let i = 9; i >= 0; --i) {
        deque.insertFront(i);
    }

    for (let i = 0; i < testArray.length; i++) {
        arktest.assertEQ(deque[i], testArray[i]);
    }

    for (let i = 10; i < 20; i++) {
        deque.insertEnd(i);
        testArray.push(i);
    }

    for (let i = 0; i < testArray.length; i++) {
        arktest.assertEQ(deque[i], testArray[i]);
    }

    deque.forEach((i, d) => {
        arktest.assertEQ(d, testArray[i]);
    });

    let j = 0;
    for (const data of deque) {
        arktest.assertEQ(data, testArray[j]);
        j++;
    }

    let itr = deque.$_iterator();
    let tmp = itr.next().value;
    let testArray1 = new Array<number>();
    do {
        testArray1.push(tmp!);
        tmp = itr.next().value;
    } while (tmp != undefined);

    for (let k = 0; k < deque.length; k++) {
        arktest.assertEQ(testArray1[k], testArray[k]);
    }

    arktest.assertTrue(deque.has(7), "Deque should contain the element 7");
    arktest.assertEQ(deque.getFirst(), 0);
    arktest.assertEQ(deque.getLast(), 19);
    arktest.assertEQ(deque.popFirst(), 0);
    arktest.assertEQ(deque.popLast(), 19);
}

function testExceptionHandlingInvalidIndex() {
    let myDeque = new Deque<number>();
    myDeque.insertEnd(1);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            `The type of \"index\" must be small integer.`) && ((e as BusinessError).code == TypeErrorCodeId);
    }
    arktest.expectThrow(() => {myDeque[2147483648]}, exceptionCheck);
}

function testPopFirstAndIteration() {
    let arr1 = new Array<number>();
    let myTest = new Deque<number>();
    for (let i = 0; i < 10; i++) {
        myTest.insertEnd(i);
    }
    for (let i = 0; i < 5; i++) {
        myTest.popFirst();
    }
    myTest.forEach((item: number, index: number, arr: Deque<number>) => {
        arr1.push(item);
    });
    arktest.assertEQ(myTest.length, 5);
    for (let j = 5; j < 10; j++) {
        arktest.assertEQ(arr1[j - 5], j);
    }
}

function testKeyIteration() {
    let test1 = new Deque<number>();
    for (let k = 0; k < 10; k++) {
        test1.insertEnd(k);
    }
    let keyName = "";
    let key: number = 0;
    for (key in test1) {
        keyName += key;
    }
    arktest.assertEQ(keyName, "0123456789");
}
