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

    suite.addTest("ArrayList clear", testArrayListClear);
    suite.addTest("ArrayList clear with empty container", testArrayListClearEmpty);
    suite.addTest("ArrayList clone with empty container", testArrayListCloneEmpty);
    suite.addTest("ArrayList clone", testArrayListClone);
    suite.addTest("ArrayList clone with change cloneArrayList", testArrayListCloneModify);
    suite.addTest("ArrayList clone after clear", testArrayListCloneAfterClear);
    suite.addTest("ArrayList getCapacity", testArrayListGetCapacity);
    suite.addTest("ArrayList getCapacity with empty container", testArrayListGetCapacityEmpty);
    suite.addTest("ArrayList getCapacity after clear", testArrayListGetCapacityAfterClear);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testNumber3: number = 3;
    static readonly testNumber6: number = 6;
    static readonly testNumber10: number = 10;
    static readonly testNumber15: number = 15;
    static readonly forNumber: Number[] = [1, 2, 3, 4, 5];
    static readonly result3: Number[] = [3, 1, 2, 3, 4, 5, 3];
    static readonly result4: Number[] = [3, 1, 2, 3, 4, 5];
}

function testArrayListClear() {
    let arrayList = new ArrayList<number>();
    arrayList.insert(TestData.testNumber1, 0);
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.subArrayList(TestData.testNumber1, TestData.testNumber3);
    let length: number = arrayList.length;
    arrayList.clear();
    assertEQ(length, TestData.testNumber6, "ArrayList length should be 6 before clear");
    assertEQ(arrayList.length, 0, "ArrayList length should be 0 after clear");
    assertTrue(arrayList.isEmpty(), "ArrayList should be empty after clear");
    assertEQ(arrayList.getCapacity(), TestData.testNumber10, "ArrayList capacity should be 10 after clear");
}

function testArrayListClearEmpty() {
    let arrayList = new ArrayList<number>();
    let length: number = arrayList.length;
    arrayList.clear();
    assertEQ(length, 0, "Initial ArrayList length should be 0");
    assertEQ(arrayList.length, 0, "ArrayList length should be 0 after clear");
}

function testArrayListCloneEmpty() {
    let arrayList = new ArrayList<number>();
    let cloneArrayList = arrayList.clone();
    assertEQ(cloneArrayList.length, 0, "Cloned ArrayList should have length 0");
}

function testArrayListClone() {
    let arrayList = new ArrayList<number>();
    arrayList.insert(TestData.testNumber1, 0);
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.subArrayList(TestData.testNumber1, TestData.testNumber3);
    let cloneArrayList = arrayList.clone();
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(cloneArrayList[i], arrayList[i], `Element at index ${i} should match`);
    }
    assertEQ(cloneArrayList.length, arrayList.length, "Cloned ArrayList should have the same length");
}

function testArrayListCloneModify() {
    let arrayList = new ArrayList<number>();
    initializeArrayList(arrayList, TestData.result4);
    let cloneArrayList = arrayList.clone();
    for (let i: number = 0; i < arrayList.length; i++) {
        assertEQ(cloneArrayList[i], arrayList[i], `Element at index ${i} should match`);
    }
    cloneArrayList.add(TestData.testNumber3);
    for (let i: int = 0; i < arrayList.length; i++) {
        assertEQ(arrayList[i], TestData.result4[i], `Element at index ${i} should match`);
    }
    for (let i: int = 0; i < cloneArrayList.length; i++) {
        assertEQ(cloneArrayList[i], TestData.result3[i], `Element at index ${i} should match`);
    }
    assertEQ(cloneArrayList.length, TestData.testNumber6 + TestData.testNumber1,
        "Cloned ArrayList should have the correct length after modification");
    assertEQ(arrayList.length, TestData.testNumber6,
        "Original ArrayList should remain unchanged after clone modification");
}

function testArrayListCloneAfterClear() {
    let arrayList = new ArrayList<number>();
    arrayList.insert(TestData.testNumber1, 0);
    initializeArrayList(arrayList, TestData.forNumber);
    arrayList.subArrayList(TestData.testNumber1, TestData.testNumber3);
    arrayList.clear();
    assertEQ(arrayList.length, 0, "ArrayList length should be 0 after clear");
    let cloneArrayList = arrayList.clone();
    assertEQ(cloneArrayList.length, arrayList.length, "Cloned ArrayList should have the same length");
}

function testArrayListGetCapacity() {
    let arrayList = new ArrayList<number>();
    arrayList.insert(TestData.testNumber1, 0);
    for (let i: number = 0; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    arrayList.remove(TestData.testNumber1);
    arrayList.subArrayList(TestData.testNumber1, TestData.testNumber3);
    let cap: number = arrayList.getCapacity();
    initializeArrayList(arrayList, TestData.forNumber);
    assertEQ(cap, TestData.testNumber10, "Initial ArrayList capacity should be 10");
    assertEQ(arrayList.getCapacity(), TestData.testNumber15, "ArrayList capacity should be updated correctly");
}

function testArrayListGetCapacityEmpty() {
    let arrayList = new ArrayList<number>();
    assertEQ(arrayList.getCapacity(), TestData.testNumber10, "Initial ArrayList capacity should be 10");
}

function testArrayListGetCapacityAfterClear() {
    let arrayList = new ArrayList<number>();
    arrayList.insert(TestData.testNumber1, 0);
    for (let i: number = 0; i < TestData.testNumber6; i++) {
        arrayList.add(i);
    }
    arrayList.remove(TestData.testNumber1);
    arrayList.subArrayList(TestData.testNumber1, TestData.testNumber3);
    let cap: number = arrayList.getCapacity();
    arrayList.clear();
    assertEQ(cap, TestData.testNumber10, "Initial ArrayList capacity should be 10");
    assertEQ(arrayList.getCapacity(), TestData.testNumber10, "ArrayList capacity should be updated correctly");
}

function initializeArrayList<T>(arrayList: ArrayList<T>, sourceArray: T[]) {
    for (let i = 0; i < sourceArray.length; i++) {
        arrayList.add(sourceArray[i]);
    }
}
