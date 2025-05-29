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

function main() {
    const suite = new ArkTestsuite("List API tests");

    suite.addTest("List index getter", testIndexGetter);
    suite.addTest("List index setter", testIndexSetter);
    suite.addTest("List index out of range", testIndexOutOfRange);

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
        assertEQ(list[i], testNumbers[i], `Element at index ${i} should match`);
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
        assertEQ(value, 5, `Element should match`);
        forEachCount++;
    });
    assertEQ(list.length, forEachCount, "List length should match forEach count");
}

function testIndexOutOfRange() {
    let list = new List<number>();
    initializeList(list, testNumbers);
    let result = list[testNumbers.length + 1];
    assertEQ(result, undefined, "The result should be undefined");
    
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof RangeError) && ((e as RangeError).message ==
            `The value of index is out of range.`)
    }
    expectThrow(() => {list[testNumbers.length + 1] = 1}, exceptionCheck);
}
