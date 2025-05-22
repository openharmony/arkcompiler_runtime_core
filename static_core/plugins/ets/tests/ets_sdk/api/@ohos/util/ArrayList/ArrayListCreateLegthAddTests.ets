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

import {ArrayList} from "@ohos.util.ArrayList";

function main() {
    const suite = new ArkTestsuite("ArrayList API tests");

    suite.addTest("ArrayList create an instance that stores single-type data", testCreateSingleTypeInstance);
    suite.addTest("ArrayList create an instance that stores multiple types of data", testCreateMultiTypeInstance);
    suite.addTest("ArrayList length", testArrayListLength);
    suite.addTest("ArrayList length with mixed types", testArrayListLengthMixedTypes);
    suite.addTest("ArrayList length setter with large value", testArrayListLengthSetterLarge);
    suite.addTest("ArrayList add", testArrayListAdd);
    suite.addTest("ArrayList add special characters", testArrayListAddSpecialCharacters);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber2: number = 2;
    static readonly testNumber3: number = 3;
    static readonly testNumber5: number = 5;
    static readonly testNumber8: number = 8;
    static readonly testNumber1w: number = 10000;
    static readonly testString1: string = "string";
    static readonly testString2: string = "";
    static readonly testString4: string = "%";
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly arrayResult9 = `{"DEFAULT_CAPACITY":10,"capacity":10,"buffer":["string","","%"]}`;
}

function testCreateSingleTypeInstance() {
    let arrayList: ArrayList<number>;
    expectNoThrow(() => {arrayList = new ArrayList<number>()});
}

function testCreateMultiTypeInstance() {
    let arrayList: ArrayList<number | string>;
    expectNoThrow(() => {arrayList = new ArrayList<number | string>()});
}

function testArrayListLength() {
    let arrayList = new ArrayList<number>();
    assertEQ(arrayList.length, 0, "Initial ArrayList length should be 0");
    arrayList.add(TestData.testNumber3);
    arrayList.add(TestData.testNumber1);
    arrayList.add(TestData.testNumber2);
    assertEQ(arrayList.length, TestData.testNumber3, "ArrayList length should be 3 after adding three elements");
    arrayList.remove(TestData.testNumber3);
    arrayList.remove(TestData.testNumber2);
    assertEQ(arrayList.length, TestData.testNumber1, "ArrayList length should be 1 after removing two elements");
}

function testArrayListLengthMixedTypes() {
    let arrayList = new ArrayList<number | string | boolean>();
    assertEQ(arrayList.length, 0, "Initial ArrayList length should be 0");
    arrayList.add(TestData.testNumber3);
    arrayList.add(TestData.testString1);
    arrayList.add(true);
    assertEQ(arrayList.length, TestData.testNumber3,
        "ArrayList length should be 3 after adding three elements of mixed types");
    arrayList.remove(TestData.testNumber3);
    arrayList.remove(TestData.testString1);
    assertEQ(arrayList.length, TestData.testNumber1, "ArrayList length should be 1 after removing two elements");
}

function testArrayListLengthSetterLarge() {
    let arrayList = new ArrayList<number>();
    for (let i: number = 0; i < TestData.testNumber1w; i++) {
        arrayList.add(TestData.testNumber1);
    }
    assertEQ(arrayList.length, TestData.testNumber1w, "ArrayList length should be 10000 after adding elements");
}

function testArrayListAdd() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.forNumber);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.forNumber[i], `Element at index ${i} should match`);
    }
    assertEQ(arrayList.length, TestData.testNumber5, "ArrayList length should be 5 after adding elements");
}

function testArrayListAddSpecialCharacters() {
    let arrayList = new ArrayList<number | string | Array<number | string | boolean> | boolean | undefined | null |
        Object>();
    arrayList.add(TestData.testString1);
    arrayList.add(TestData.testString2);
    arrayList.add(TestData.testString4);
    assertEQ(JSON.stringify(arrayList), TestData.arrayResult9, "Element should be equal");
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
