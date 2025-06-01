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

    suite.addTest("PlainArray getKeyAt", testGetKeyAtValidIndex);
    suite.addTest("PlainArray getKeyAt key by boundary index", testGetKeyAtBoundaryIndex);
    suite.addTest("PlainArray getKeyAt key by exceeded index", testGetKeyAtExceededIndex);
    suite.addTest("PlainArray getKeyAt with empty container", testGetKeyAtIndexOfEmpty);
    suite.addTest("PlainArray getValueAt", testGetValueAtValidIndex);
    suite.addTest("PlainArray getValueAt key by boundary index", testGetValueAtBoundaryIndex);
    suite.addTest("PlainArray getValueAt key by exceeded index", testGetValueAtExceededIndex);
    suite.addTest("PlainArray getValueAt with empty container", testGetValueAtWithEmpty);

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
    static readonly testNumber4: number = 4;
    static readonly testNumber6: number = 6;
    static readonly forNumber2: Number[] = [0, 2, 2, 3, 4];
}

function testGetKeyAtValidIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.getKeyAt(TestData.testNumber1), TestData.testNumber1,
        "The plainArray should return the correct key at the specified index");
}

function testGetKeyAtBoundaryIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.getKeyAt(0), 0, "The plainArray should return the correct key at index 0");
    assertEQ(plainArray.getKeyAt(TestData.forNumber2.length - TestData.testNumber1), TestData.testNumber4,
        "The plainArray should return the correct key at index length - 1");
}

function testGetKeyAtExceededIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let count = 0;
    try {
        plainArray.getKeyAt(-TestData.testNumber1);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        count++;
    }
    try {
        plainArray.getKeyAt(TestData.testNumber6);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        count++;
    }
    assertEQ(count, TestData.testNumber2, "The result should be 'RangeError'");
}

function testGetKeyAtIndexOfEmpty() {
    let plainArray = new PlainArray<number>();
    try {
        plainArray.getKeyAt(0);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertTrue(false, "The result should be 'RangeError'");
}

function testGetValueAtValidIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.getValueAt(TestData.testNumber1), TestData.testNumber2,
        "The plainArray should return the correct value at the specified index");
}

function testGetValueAtBoundaryIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.getValueAt(0), 0, "The plainArray should return the correct value at index 0");
    assertEQ(plainArray.getValueAt(TestData.forNumber2.length - TestData.testNumber1), TestData.testNumber4,
        "The plainArray should return the correct value at index length - 1");
}

function testGetValueAtExceededIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let count = 0;
    try {
        plainArray.getValueAt(-TestData.testNumber1);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        count++;
    }
    try {
        plainArray.getValueAt(TestData.testNumber6);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        count++;
    }
    assertEQ(count, TestData.testNumber2, "The result should be 'RangeError'");
}

function testGetValueAtWithEmpty() {
    let plainArray = new PlainArray<number>();
    try {
        plainArray.getValueAt(0);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertTrue(false, "The result should be 'RangeError'");
}