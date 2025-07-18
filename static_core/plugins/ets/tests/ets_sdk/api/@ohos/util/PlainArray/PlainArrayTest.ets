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

import { PlainArray } from "@ohos.util.PlainArray";
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: number = 10200001;

// Test cases ported from ArkTS 1.0: container_plainarray.js
function main() {
    const suite = new arktest.ArkTestsuite("PlainArray API tests");

    suite.addTest("Testing PlainArray basic operations", testPlainArrayBasicOperations);
    suite.addTest("Testing PlainArray setValueAt and clear operations", testSetValueAtAndClearOperations);
    suite.addTest("Testing PlainArray exception handling for empty container", testExceptionHandlingForEmptyContainer);
    suite.addTest("Testing PlainArray operations with Math.floor keys", testOperationsWithMathFloorKeys);

    return suite.run();
}

function testPlainArrayBasicOperations() {
    let plainArray = new PlainArray<string>();
    let array = ["0", "1", "2", "3", "4", "5"];
    let testArray = Array.from<string>(array);

    plainArray.add(0, "0");
    plainArray.add(1, "1");
    plainArray.add(2, "2");
    plainArray.add(3, "3");
    plainArray.add(4, "4");
    plainArray.add(5, "5");

    for (let i = 0; i < testArray.length; i++) {
        arktest.assertEQ(plainArray.getValueAt(i), testArray[i]);
    }

    arktest.assertEQ(plainArray.length, 6);
    arktest.assertTrue(plainArray.has(2), "The PlainArray should contain the key 2");
    arktest.assertEQ(plainArray.getIndexOfValue("1"), 1);
    arktest.assertEQ(plainArray.getIndexOfKey(5), 5);
    arktest.assertEQ(plainArray.getKeyAt(1), 1);
    arktest.assertEQ(plainArray.getValueAt(2), "2");
    
    let newPlainArray = plainArray.clone();
    for (let i = 0; i < testArray.length; i++) {
        arktest.assertEQ(newPlainArray.getValueAt(i), testArray[i]);
    }

    const removeRes = plainArray.remove(3);
    testArray.splice(3.0, 1.0);
    arktest.assertEQ(removeRes, "3");

    plainArray.removeAt(2);
    testArray.splice(2.0, 1.0);

    for (let i = 0; i < testArray.length; i++) {
        arktest.assertEQ(plainArray.getValueAt(i), testArray[i]);
    }

    newPlainArray = plainArray.clone();
    let res1 = plainArray.removeRangeFrom(1, 2);
    if (res1 > 0) {
        res1 = newPlainArray.removeRangeFrom(0, 100);
        if (res1 > 0) {
            arktest.assertTrue(newPlainArray.isEmpty(), "The PlainArray should be empty after removing all elements");
        }
    }

    plainArray.forEach((i, d) => {});

    for (const data of plainArray) {}

    let itr = plainArray.$_iterator();
    let tmp: IteratorResult<[Number, string]> = itr.next();
    let i = 0;
    while (!tmp.done) {
        arktest.assertEQ(tmp.value![1], plainArray.getValueAt(i));
        tmp = itr.next();
        i++;
    }
}

function testSetValueAtAndClearOperations() {
    let arr2 = new PlainArray<string>();
    arr2.add(0, "0");
    arr2.add(1, "1");
    arr2.add(2, "2");
    arr2.add(3, "3");
    arr2.add(4, "4");
    arr2.add(5, "5");
    arr2.setValueAt(2, "123");
    arktest.assertEQ(arr2.get(2), "123");
    arr2.clear();
    arktest.assertEQ(arr2.length, 0);
    arktest.assertTrue(arr2.isEmpty(), "The PlainArray should be empty after clearing");
    arr2.add(0, "0");
    arr2.add(1, "1");
    arr2.add(2, "2");
    arr2.add(3, "3");
    arr2.add(4, "4");
    arr2.add(5, "5");
    arktest.assertEQ(arr2.toString(), "0:0,1:1,2:2,3:3,4:4,5:5");
}

function testExceptionHandlingForEmptyContainer() {
    let empty_pa = new PlainArray<number>();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message == "Container is empty") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {empty_pa.getValueAt(0)}, exceptionCheck);
    let exceptionCheck1 = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message == "Container is empty") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {empty_pa.removeRangeFrom(0, 1)}, exceptionCheck1);
    let exceptionCheck2 = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message == "Container is empty") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {empty_pa.setValueAt(0, 1)}, exceptionCheck2);
}

function testOperationsWithMathFloorKeys() {
    let myPa = new PlainArray<string>();
    myPa.add(0, "a");
    myPa.add(Math.floor(1.5), "b");
    arktest.assertTrue(myPa.has(Math.floor(1.5)), "The PlainArray should contain the key Math.floor(1.5)");
    arktest.assertEQ(myPa.get(Math.floor(1.5)), "b");
    arktest.assertEQ(myPa.getValueAt(Math.floor(1.5)), "b");
    myPa.setValueAt(Math.floor(1.5), "c");
    arktest.assertEQ(myPa.getKeyAt(Math.floor(1.5)), 1);
    arktest.assertEQ(myPa.getIndexOfKey(Math.floor(1.5)), 1);
    myPa.removeAt(Math.floor(1.5));
    arktest.assertEQ(myPa.length, 1);
}
