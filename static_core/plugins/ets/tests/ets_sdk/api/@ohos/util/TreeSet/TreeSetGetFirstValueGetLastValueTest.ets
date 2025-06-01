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

import {TreeSet} from "@ohos.util.TreeSet";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("TreeSet GetFirstValue GetLastValue API tests")

    suite.addTest("TreeSet getFirstValue getLastVallue first and last value",
        testGetFirstValueGetLastValueSingleElement)
    suite.addTest("TreeSet getFirstValue getLastVallue first and last value from the null treeSet",
        testGetFirstValueGetLastValueEmptyTreeSet)
    suite.addTest("TreeSet getFirstValue getLastVallue first and last value from the TreeSet with a set of data",
        testGetFirstValueGetLastValueMultipleElements)
    suite.addTest("TreeSet getFirstValue getLastVallue first and last value by descending",
        testGetFirstValueGetLastValueDescendingOrder)
    suite.addTest("TreeSet getFirstValue getLastVallue first and last object value",
        testGetFirstValueGetLastValueObjectValues)
    suite.addTest("TreeSet getFirstValue getLastVallue first and last object value from the TreeSet with a set of data",
        testGetFirstValueGetLastValueMultipleObjectValues)

    return suite.run()
}

class MyObject extends Object {
    size: number;
    constructor(size: number) {
        super();
        this.size = size;
    }
}

function testGetFirstValueGetLastValueSingleElement() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    assertEQ(result1, "aa", "The treeSet should return the correct first value");
    assertEQ(result2, "bb", "The treeSet should return the correct last value");
    assertEQ(treeSet.length, 2, "The treeSet length should be 2")
}

function testGetFirstValueGetLastValueEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    assertEQ(result1, undefined, "The treeSet should return undefined for the first value in an empty set");
    assertEQ(result2, undefined, "The treeSet should return undefined for the last value in an empty set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0")
}

function testGetFirstValueGetLastValueMultipleElements() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    assertEQ(result1, "aa", "The treeSet should return the correct first value");
    assertEQ(result2, "aa", "The treeSet should return the correct last value");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1")
}

function testGetFirstValueGetLastValueDescendingOrder() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return firstValue.compareTo(secondValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.add("cc");
    treeSet.add("dd");
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    assertEQ(result1, "dd", "The treeSet should return the correct first value");
    assertEQ(result2, "aa", "The treeSet should return the correct last value");
    assertEQ(treeSet.length, 4, "The treeSet length should be 4")
}

function testGetFirstValueGetLastValueObjectValues() {
    let treeSet: TreeSet<MyObject> =
        new TreeSet<MyObject>((firstValue: MyObject, secondValue: MyObject): number => {
            return secondValue.size - firstValue.size;
        });
    let obj: MyObject = new MyObject(1);
    let obj_sec: MyObject = new MyObject(2);
    let obj_trd: MyObject = new MyObject(3);
    treeSet.add(obj);
    treeSet.add(obj_sec);
    treeSet.add(obj_trd);
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    assertEQ(result1, obj, "The treeSet should return the correct first value");
    assertEQ(result2, obj_trd, "The treeSet should return the correct last value");
    assertEQ(treeSet.length, 3, "The treeSet length should be 3")
}

function testGetFirstValueGetLastValueMultipleObjectValues() {
    let treeSet: TreeSet<MyObject> =
        new TreeSet<MyObject>((firstValue: MyObject, secondValue: MyObject): number => {
            return secondValue.size - firstValue.size;
        });
    let obj: MyObject = new MyObject(1);
    treeSet.add(obj);
    let result1 = treeSet.getFirstValue();
    let result2 = treeSet.getLastValue();
    assertEQ(result1, obj, "The treeSet should return the correct first value");
    assertEQ(result2, obj, "The treeSet should return the correct last value");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1")
}
