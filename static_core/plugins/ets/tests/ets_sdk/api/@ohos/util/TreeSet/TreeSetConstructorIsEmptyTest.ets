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
    const suite = new ArkTestsuite("TreeSet Constructor IsEmpty API tests")

    suite.addTest("TreeSet construct default empty string treeSet", testConstructDefaultEmptyStringTreeSet)
    suite.addTest("TreeSet construct default empty number treeSet", testConstructDefaultEmptyNumberTreeSet)
    suite.addTest("TreeSet construct default empty object treeSet", testConstructDefaultEmptyObjectTreeSet)
    suite.addTest("TreeSet construct no comparator treeSet", testConstructNoComparatorTreeSet)
    suite.addTest("TreeSet isEmpty is true", testIsEmptyReturnsTrue)
    suite.addTest("TreeSet isEmpty is false", testIsEmptyReturnsFalse)
    suite.addTest("TreeSet isEmpty is true after remove data", testIsEmptyReturnsTrueAfterRemove)

    return suite.run()
}

class MyObject extends Object {
    size: number;
    constructor(size: number) {
        super();
        this.size = size;
    }
}

function testConstructDefaultEmptyStringTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    assertTrue(treeSet instanceof TreeSet, "The treeSet should be of the same type as the TreeSet");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function testConstructDefaultEmptyNumberTreeSet() {
    let treeSet: TreeSet<number> = new TreeSet<number>((firstValue: number,
        secondValue: number): number => { return secondValue - firstValue });
    treeSet.add(111);
    treeSet.add(222);
    treeSet.add(333);
    assertTrue(treeSet instanceof TreeSet, "The treeSet should be of the same type as the TreeSet");
    assertEQ(treeSet.length, 3, "The treeSet length should be 3");
}

function testConstructDefaultEmptyObjectTreeSet() {
    let treeSet: TreeSet<MyObject> =
        new TreeSet<MyObject>((firstValue: MyObject, secondValue: MyObject): number => {
            return secondValue.size - firstValue.size;
        });
    let obj: MyObject = new MyObject(1);
    treeSet.add(obj);
    assertTrue(treeSet instanceof TreeSet, "The treeSet should be of the same type as the TreeSet");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function testConstructNoComparatorTreeSet() {
    let treeSet: TreeSet<number> = new TreeSet<number>();
    treeSet.add(111);
    treeSet.add(222);
    treeSet.add(333);
    assertTrue(treeSet instanceof TreeSet, "The treeSet should be of the same type as the TreeSet");
    assertEQ(treeSet.length, 3, "The treeSet length should be 3");
}

function testIsEmptyReturnsTrue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let result = treeSet.isEmpty();
    assertEQ(result, true, "The treeSet should be empty");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testIsEmptyReturnsFalse() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result = treeSet.isEmpty();
    assertEQ(result, false, "The treeSet should not be empty");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function testIsEmptyReturnsTrueAfterRemove() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.remove("aa");
    let result = treeSet.isEmpty();
    assertEQ(result, true, "The treeSet should be empty");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}
