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

import { PlainArray } from "@ohos.util.PlainArray";

function main(): int {
    const suite = new ArkTestsuite("PlainArray API tests");

    suite.addTest("PlainArray get", testGetWithExisting);
    suite.addTest("PlainArray get with inexistent key", testGetWithNonExisting);
    suite.addTest("PlainArray getIndexOfKey", testGetIndexOfKeyWithExisting);
    suite.addTest("PlainArray getIndexOfKey with inexistent key", testGetIndexOfKeyWithNonExisting);
    suite.addTest("PlainArray getIndexOfValue", testGetIndexOfValueWithExisting);
    suite.addTest("PlainArray getIndexOfValue with inexistent value", testGetIndexOfValueWithNonExisting);
    suite.addTest("PlainArray getIndexOfValue with repeated value", testGetIndexOfValueWithRepeating);

    return suite.run();
}

function initializePlainArray<T>(plainArray: PlainArray<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        plainArray.add(i, sourceArray[i]);
    }
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber6: number = 6;
    static readonly testNumber1w: number = 10000;
    static readonly forNumber1: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20];
    static readonly forNumber2: Number[] = [0, 2, 2, 3, 4];
}

function testGetWithExisting() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber1);
    for (let i = 0; i < plainArray.length; i++) {
        assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        assertEQ(plainArray.getValueAt(i), TestData.forNumber1[i], "The value should be exist");
    }
    assertEQ(plainArray.get(TestData.testNumber1), TestData.testNumber1, "The value should be 1");
}

function testGetWithNonExisting() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber1);
    for (let i = 0; i < plainArray.length; i++) {
        assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        assertEQ(plainArray.getValueAt(i), TestData.forNumber1[i], "The value should be exist");
    }
    assertEQ(plainArray.get(TestData.testNumber1w), undefined, "The result should be undefined");
}

function testGetIndexOfKeyWithExisting() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    for (let i = 0; i < plainArray.length; i++) {
        assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        assertEQ(plainArray.getValueAt(i), TestData.forNumber2[i], "The value should be exist");
    }
    assertEQ(plainArray.getIndexOfKey(TestData.testNumber1), TestData.testNumber1, "The index of key 1 should be 1");
}

function testGetIndexOfKeyWithNonExisting() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    for (let i = 0; i < plainArray.length; i++) {
        assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        assertEQ(plainArray.getValueAt(i), TestData.forNumber2[i], "The value should be exist");
    }
    assertEQ(plainArray.getIndexOfKey(TestData.testNumber6), -TestData.testNumber1, "The key 6 should be not exist");
}

function testGetIndexOfValueWithExisting() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    for (let i = 0; i < plainArray.length; i++) {
        assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        assertEQ(plainArray.getValueAt(i), TestData.forNumber2[i], "The value should be exist");
    }
    assertEQ(plainArray.getIndexOfValue(TestData.testNumber3), TestData.testNumber3,
        "The index of value 3 should be 3");
}

function testGetIndexOfValueWithNonExisting() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    for (let i = 0; i < plainArray.length; i++) {
        assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        assertEQ(plainArray.getValueAt(i), TestData.forNumber2[i], "The value should be exist");
    }
    assertEQ(plainArray.getIndexOfValue(TestData.testNumber6), -TestData.testNumber1,
        "The value 6 should be not exist");
}

function testGetIndexOfValueWithRepeating() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    for (let i = 0; i < plainArray.length; i++) {
        assertEQ(plainArray.getKeyAt(i), i, "The key should be exist");
        assertEQ(plainArray.getValueAt(i), TestData.forNumber2[i], "The value should be exist");
    }
    assertEQ(plainArray.getIndexOfValue(TestData.testNumber2), TestData.testNumber1,
        "The index of value 2 should be 1");
}
