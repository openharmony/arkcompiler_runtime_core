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

    suite.addTest("PlainArray clone non-empty container", testCloneWithNonEmpty);
    suite.addTest("PlainArray clone empty container", testCloneWithEmpty);
    suite.addTest("PlainArray clone with changing clone container", testCloneWithChanging);
    suite.addTest("PlainArray add to single-type container", testAddSingleType);
    suite.addTest("PlainArray add to composite-type container", testAddCompositeType);
    suite.addTest("PlainArray add same key", testAddSameKey);
    suite.addTest("PlainArray add same value", testAddSameValue);
    suite.addTest("PlainArray add special number", testAddSpecialNumber);

    return suite.run();
}

function initializePlainArray<T>(plainArray: PlainArray<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        plainArray.add(i, sourceArray[i]);
    }
}

class Tmp {
  age: number = 0;

  constructor(age: number) {
    this.age = age;
  }
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber4: number = 4;
    static readonly testNumber5: number = 5;
    static readonly testNumber6: number = 6;
    static readonly testNumberMax: number = 2147483647;
    static readonly testNumberMin: number = -2147483648;
    static readonly testString1: string = "squirrel";
    static readonly testString2: string = "1:1,2:squirrel,3:false,4:[object Object],5:null,6:undefined";
    static readonly testString3: string = "1:1,2:1";
    static readonly testObject1 = new Tmp(10);
    static readonly forNumber2: Number[] = [0, 2, 2, 3, 4];
}

function testCloneWithNonEmpty() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let clonePlainArray = plainArray.clone();
    assertEQ(plainArray.toString(), clonePlainArray.toString(), "The elements of both containers should be same");
}

function testCloneWithEmpty() {
    let plainArray = new PlainArray<number>();
    let clonePlainArray = plainArray.clone();
    assertEQ(plainArray.length, clonePlainArray.length, "The length should be same as original plainArray");
    assertEQ(clonePlainArray.length, 0, "The length should be 0");
}

function testCloneWithChanging() {
    let plainArray = new PlainArray<number>();
    initializePlainArray(plainArray, TestData.forNumber2);
    let clonePlainArray = plainArray.clone();
    assertEQ(plainArray.toString(), clonePlainArray.toString(), "The elements of both containers should be same");
    clonePlainArray.add(TestData.testNumber6, TestData.testNumber1);
    assertEQ(clonePlainArray.getKeyAt(TestData.testNumber5), TestData.testNumber6, "The key should be added");
    assertEQ(clonePlainArray.getValueAt(TestData.testNumber5), TestData.testNumber1, "The value should be added");
    assertEQ(plainArray.length, TestData.forNumber2.length, "The original plainArray length should not change");
}

function testAddSingleType() {
    let plainArray = new PlainArray<number>();
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    assertEQ(plainArray.length, TestData.testNumber1, "PlainArray length should be 1");
}

function testAddCompositeType() {
    let plainArray = new PlainArray<number | string | boolean | object | null | undefined>();
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    plainArray.add(TestData.testNumber2, TestData.testString1);
    plainArray.add(TestData.testNumber3, false);
    plainArray.add(TestData.testNumber4, TestData.testObject1);
    plainArray.add(TestData.testNumber5, null);
    plainArray.add(TestData.testNumber6, undefined);
    console.log(plainArray.toString());
    assertEQ(plainArray.length, TestData.testNumber6, "PlainArray length should be 6");
    assertEQ(plainArray.toString(), TestData.testString2, "The elements should be added");
}

function testAddSameKey() {
    let plainArray = new PlainArray<number>();
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    plainArray.add(TestData.testNumber1, TestData.testNumber2);
    assertEQ(plainArray.length, TestData.testNumber1, "PlainArray length should be 1");
    assertEQ(plainArray.get(TestData.testNumber1), TestData.testNumber2,
        "The value corresponding to key 1 should be 2");
}

function testAddSameValue() {
    let plainArray = new PlainArray<number>();
    plainArray.add(TestData.testNumber1, TestData.testNumber1);
    plainArray.add(TestData.testNumber2, TestData.testNumber1);
    assertEQ(plainArray.length, TestData.testNumber2, "PlainArray length should be 2");
    assertEQ(plainArray.toString(), TestData.testString3, "The elements should be added");
}

function testAddSpecialNumber() {
    let plainArray = new PlainArray<number>();
    plainArray.add(-TestData.testNumber1, TestData.testNumber1);
    plainArray.add(0, TestData.testNumber1);
    plainArray.add(TestData.testNumberMax, TestData.testNumber1);
    plainArray.add(TestData.testNumberMin, TestData.testNumber1);
    assertTrue(plainArray.has(-TestData.testNumber1), "The key should be added");
    assertTrue(plainArray.has(0), "The key should be added");
    assertTrue(plainArray.has(TestData.testNumberMax), "The key should be added");
    assertTrue(plainArray.has(TestData.testNumberMin), "The key should be added");
}