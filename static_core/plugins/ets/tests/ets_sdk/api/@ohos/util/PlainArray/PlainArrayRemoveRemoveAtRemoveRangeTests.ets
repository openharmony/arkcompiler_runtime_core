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

    suite.addTest("PlainArray remove with exist key", testRemoveWithExistKey);
    suite.addTest("PlainArray remove with non-exist key", testRemoveWithNonExistKey);
    suite.addTest("PlainArray remove empty container", testRemoveWithEmpty);
    suite.addTest("PlainArray removeAt by valid index", testRemoveAtValidIndex);
    suite.addTest("PlainArray removeAt by boundary index", testRemoveAtBoundaryIndex);
    suite.addTest("PlainArray removeAt by exceeded index", testRemoveAtExceededIndex);
    suite.addTest("PlainArray removeAt empty container", testRemoveAtWithEmpty);
    suite.addTest("PlainArray removeRangeFrom by valid range", testRangeFromValidRange);
    suite.addTest("PlainArray removeRangeFrom by exceeded range", testRangeFromExceededRange);
    suite.addTest("PlainArray removeRangeFrom by exceeded index", testRangeFromExceededIndex);
    suite.addTest("PlainArray removeRangeFrom empty container", testRangeFromWithEmpty);
    suite.addTest("PlainArray removeRangeFrom with double", testRangeFromWithDouble);

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
    static readonly testNumber4: number = 4;
    static readonly testNumber6: number = 6;
    static readonly testString1: string = "0:0,1:2,2:2,4:4";
    static readonly testString2: string = "0:0,1:2,2:2,3:3,4:4";
    static readonly testString3: string = "1:2,2:2,3:3";
    static readonly testString4: string = "0:0,1:2,4:4";
    static readonly testString5: string = "0:0,1:2";
    static readonly testDouble: double = 7.5;
    static readonly forNumber2: Number[] = [0, 2, 2, 3, 4];
}

function testRemoveWithExistKey() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.remove(TestData.testNumber3), TestData.testNumber3, "The result should be removed value");
    assertEQ(plainArray.toString(), TestData.testString1, "The removed sequence is normal");
}

function testRemoveWithNonExistKey() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.remove(TestData.testNumber6), undefined,
        "The result should be undefined when key is not exist");
    assertEQ(plainArray.toString(), TestData.testString2, "The sequence should not change");
}

function testRemoveWithEmpty() {
    let plainArray = new PlainArray<number>();
    assertEQ(plainArray.remove(TestData.testNumber1), undefined,
        "The result should be undefined when container is empty");
    assertEQ(plainArray.length, 0, "The sequence should not change");
}

function testRemoveAtValidIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.removeAt(TestData.testNumber3), TestData.testNumber3, "The result should be removed value");
    assertEQ(plainArray.toString(), TestData.testString1, "The removed sequence is normal");
}

function testRemoveAtBoundaryIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.removeAt(TestData.testNumber4), TestData.testNumber4, "The result should be removed value");
    assertEQ(plainArray.removeAt(0), 0, "The result should be removed value");
    assertEQ(plainArray.toString(), TestData.testString3, "The removed sequence is normal");
}

function testRemoveAtExceededIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let count = 0;
    try {
        plainArray.removeAt(-TestData.testNumber1);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        count++;
    }
    try {
        plainArray.removeAt(TestData.testNumber6);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        count++;
    }
    assertEQ(count, TestData.testNumber2, "The result should be 'RangeError'");
}

function testRemoveAtWithEmpty() {
    let plainArray = new PlainArray<number>();
    try {
        plainArray.removeAt(0);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertTrue(false, "The result should be 'RangeError'");
}

function testRangeFromValidRange() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.removeRangeFrom(TestData.testNumber2, TestData.testNumber2), TestData.testNumber2,
        "The result should be 2");
    assertEQ(plainArray.toString(), TestData.testString4, "The removed sequence is normal");
}

function testRangeFromExceededRange() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.removeRangeFrom(TestData.testNumber2, TestData.testNumber6), TestData.testNumber3,
        "The result should be 3");
    assertEQ(plainArray.toString(), TestData.testString5, "The removed sequence is normal");
}

function testRangeFromExceededIndex() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let count = 0;
    try {
        plainArray.removeRangeFrom(-TestData.testNumber2, TestData.testNumber2);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        count++;
    }
    try {
        plainArray.removeRangeFrom(TestData.testNumber6, TestData.testNumber2);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        count++;
    }
    assertEQ(count, TestData.testNumber2, "The result should be 'RangeError'");
}

function testRangeFromWithEmpty() {
    let plainArray = new PlainArray<number>();
    try {
        plainArray.removeRangeFrom(0, TestData.testNumber2);
    } catch (e) {
        assertTrue(e instanceof RangeError, "The result should be 'RangeError'");
        assertEQ((e as RangeError).message, "The value of index is out of range.",
            "The result should be 'Index out of range'");
        return;
    }
    assertTrue(false, "The result should be 'RangeError'");
}

function testRangeFromWithDouble() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    assertEQ(plainArray.removeRangeFrom(TestData.testNumber2, Math.floor(TestData.testDouble)), TestData.testNumber3,
        "The result should be 3");
    assertEQ(plainArray.toString(), TestData.testString5, "The removed sequence is normal");
}