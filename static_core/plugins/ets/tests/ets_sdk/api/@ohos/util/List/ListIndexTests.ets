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

import { List } from '@ohos.util.List';
import {BusinessError} from "@ohos.base";

const OutOfBoundsErrorCodeId: number = 10200001;

function main() {
    const suite = new arktest.ArkTestsuite("List API tests");

    suite.addTest("List index getter", testIndexGetter);
    suite.addTest("List index setter", testIndexSetter);
    suite.addTest("List index out of range", testIndexOutOfRange);
    suite.addTest("List index input negative", testIndexInputNegative);
    suite.addTest("List empty getter out of range", testEmptyGetterOutOfRange);
    suite.addTest("List getter out of range", testGetterOutOfRange);

    return suite.run();
}

const testNumbers: Number[] = [45, 89, 12, 1, 34, 90, 56, 78, 23, 5, 81, 60,
    19, 41, 73, 9, 50, 38, 6, 77, 28];

function initializeList(list: List<number>, sourceArray: Number[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        list.add(sourceArray[i]);
    }
}

function testIndexGetter() {
    let list = new List<number>();
    initializeList(list, testNumbers);
    for (let i: int = 0; i < list.length; i++) {
        arktest.assertEQ(list[i], testNumbers[i], `Element at index ${i} should match`);
    }
}

function testIndexSetter() {
    let list = new List<number>();
    initializeList(list, testNumbers);
    for (let i: number = 0; i < list.length; i++) {
        list[i] = 5;
    }
    let forEachCount = 0;
    list.forEach((value: number) => {
        arktest.assertEQ(value, 5, `Element should match`);
        forEachCount++;
    });
    arktest.assertEQ(list.length, forEachCount, "List length should match forEach count");
}

function testIndexOutOfRange() {
    let list = new List<number>();
    initializeList(list, testNumbers);

    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= 20. Received value is: 22") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {list[testNumbers.length + 1] = 1}, exceptionCheck);
}

function testIndexInputNegative() {
    let list = new List<number>();
    arktest.expectNoThrow(() => {list[-1] = 1});

    list.add(1);
    arktest.expectNoThrow(() => {list[-1] = 1});
}

function testEmptyGetterOutOfRange() {
    let list = new List<number>();

    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= -1. Received value is: 10000") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {let a = list[10000]}, exceptionCheck);

    let exceptionCheck1 = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= -1. Received value is: -1") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {let a = list[-1]}, exceptionCheck1);
}

function testGetterOutOfRange() {
    let list = new List<number>();
    initializeList(list, testNumbers);

    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= 20. Received value is: 10000") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {let a = list[10000]}, exceptionCheck);

    let exceptionCheck1 = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).message ==
            "The value of \"index\" is out of range. It must be >= 0 && <= 20. Received value is: -1") &&
            ((e as BusinessError).code == OutOfBoundsErrorCodeId);
    }
    arktest.expectThrow(() => {let a = list[-1]}, exceptionCheck1);
}
