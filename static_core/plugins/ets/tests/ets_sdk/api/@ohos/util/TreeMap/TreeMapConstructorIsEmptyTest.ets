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

import TreeMap from "@ohos.util.TreeMap";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("TreeMap Constructor IsEmpty API tests")

    suite.addTest("TreeMap construct default empty string treeMap", testConstructDefaultEmptyStringTreeMap)
    suite.addTest("TreeMap construct default empty number treeMap", testConstructDefaultEmptyNumberTreeMap)
    suite.addTest("TreeMap construct default empty object treeMap", testConstructDefaultEmptyObjectTreeMap)
    suite.addTest("TreeMap construct no comparator treeMap", testConstructNoComparatorTreeMap)
    suite.addTest("TreeMap isEmpty is true", testIsEmptyReturnsTrue)
    suite.addTest("TreeMap isEmpty is false", testIsEmptyReturnsFalse)
    suite.addTest("TreeMap isEmpty is true after remove data", testIsEmptyReturnsTrueAfterRemove)

    return suite.run()
}

class MyObject extends Object {
    size: number;
    constructor(size: number) {
        super();
        this.size = size;
    }
}

function testConstructDefaultEmptyStringTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    assertTrue(treeMap instanceof TreeMap, "The treeMap should be of the same type as the TreeMap");
    assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testConstructDefaultEmptyNumberTreeMap() {
    let treeMap: TreeMap<number, number> =
        new TreeMap<number, number>((firstValue: number, secondValue: number): number => {
            return secondValue - firstValue
        });
    treeMap.set(111, 999)
    treeMap.set(222, 888)
    treeMap.set(333, 777)
    assertTrue(treeMap instanceof TreeMap, "The treeMap should be of the same type as the TreeMap");
    assertEQ(treeMap.length, 3, "The treeMap length should be 3");
}

function testConstructDefaultEmptyObjectTreeMap() {
    let treeMapo: TreeMap<MyObject, MyObject> =
        new TreeMap<MyObject, MyObject>((firstValue: MyObject, secondValue: MyObject): number => {
            return secondValue.size - firstValue.size
        });
    let obj: MyObject = new MyObject(1);
    let obj2: MyObject = new MyObject(2);
    treeMapo.set(obj, obj2);
    assertTrue(treeMapo instanceof TreeMap, "The treeMap should be of the same type as the TreeMap");
    assertEQ(treeMapo.length, 1, "The treeMap length should be 1");
}

function testConstructNoComparatorTreeMap() {
    let treeMap: TreeMap<number, number> = new TreeMap<number, number>();
    treeMap.set(111, 999)
    treeMap.set(222, 888)
    treeMap.set(333, 777)
    assertTrue(treeMap instanceof TreeMap, "The treeMap should be of the same type as the TreeMap");
    assertEQ(treeMap.length, 3, "The treeMap length should be 3");
}

function testIsEmptyReturnsTrue() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    let result = treeMap.isEmpty();
    assertEQ(result, true, "The treeMap should be empty");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testIsEmptyReturnsFalse() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result = treeMap.isEmpty();
    assertEQ(result, false, "The treeMap should not be empty");
    assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testIsEmptyReturnsTrueAfterRemove() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    let result1 = treeMap.isEmpty();
    treeMap.remove("aa");
    let result2 = treeMap.isEmpty();
    assertEQ(result1, false, "The treeMap should not be empty");
    assertEQ(result2, true, "The treeMap should be empty");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}
