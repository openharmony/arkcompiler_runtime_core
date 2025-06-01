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
    const suite = new ArkTestsuite("TreeSet GetLowerValue GetHigherValue API tests")

    suite.addTest("TreeSet getLowerValue getHigherValue lower and higher value",
        testGetLowerValueGetHigherValueValidValues)
    suite.addTest("TreeSet getLowerValue getHigherValue lower and higher value from the null treeSet",
        testGetLowerValueGetHigherValueEmptyTreeSet)
    suite.addTest("TreeSet getLowerValue getHigherValue lower and higher value with boundary value",
        testGetLowerValueGetHigherValueBoundaryValues)
    suite.addTest("TreeMap getLowerValue getHigherValue lower and higher non-exist value.",
        testGetLowerValueAndHigherValueNonExistingValue)
    suite.addTest("TreeSet getLowerValue getHigherValue lower and higher empty string value when treeSet is empty",
        testGetLowerValueGetHigherValueEmptyStringValue)
    suite.addTest("TreeSet getLowerValue getHigherValue lower and higher value by descending",
        testGetLowerValueGetHigherValueDescendingOrder)
    suite.addTest("TreeSet getLowerValue getHigherValue lower and higher object value",
        testGetLowerValueGetHigherValueObjectValues)
    suite.addTest("TreeSet getLowerValue getHigherValue lower and higher object value with boundary value",
        testGetLowerValueGetHigherValueObjectValuesBoundaryValues)

    return suite.run()
}

class MyObject extends Object {
    size: number;
    constructor(size: number) {
        super();
        this.size = size;
    }
}

function equalObjects(first: MyObject, second: MyObject): boolean {
    if (first.size == second.size) {
        return true;
    }
    return false;
}

function testGetLowerValueGetHigherValueValidValues() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.add("cc");
    let lower = treeSet.getLowerValue("bb");
    let lower_ = treeSet.getLowerValue("cc");
    let higher = treeSet.getHigherValue("aa");
    let higher_ = treeSet.getHigherValue("bb");
    assertEQ(lower, "aa", "The treeSet should return the correct lower value");
    assertEQ(lower_, "bb", "The treeSet should return the correct lower value");
    assertEQ(higher, "bb", "The treeSet should return the correct higher value");
    assertEQ(higher_, "cc", "The treeSet should return the correct higher value");
    assertEQ(treeSet.length, 3, "The treeSet length should be 3")
}

function testGetLowerValueGetHigherValueEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let lower = treeSet.getLowerValue("bb");
    let higher = treeSet.getHigherValue("aa");
    assertEQ(lower, undefined, "The treeSet should return undefined for the lower value in an empty set");
    assertEQ(higher, undefined, "The treeSet should return undefined for the higher value in an empty set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0")
}

function testGetLowerValueGetHigherValueBoundaryValues() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.add("cc");
    let lower = treeSet.getLowerValue("aa");
    let higher = treeSet.getHigherValue("cc");
    assertEQ(lower, undefined, "The treeSet should return undefined for the lower value at the boundary value");
    assertEQ(higher, undefined, "The treeSet should return undefined for the higher value at the boundary value");
    assertEQ(treeSet.length, 3, "The treeSet length should be 3")
}

function testGetLowerValueAndHigherValueNonExistingValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("bb");
    treeSet.add("cc");
    treeSet.add("dd");
    treeSet.add("ee");
    treeSet.add("gg");
    treeSet.add("ii");
    treeSet.add("kk");
    treeSet.add("nn");
    let lower_ = treeSet.getLowerValue("aa");
    let lower = treeSet.getLowerValue("ff");
    let higher_ = treeSet.getHigherValue("pp");
    let higher = treeSet.getHigherValue("ff");
    assertEQ(lower_, undefined, "The treeSet should return undefined when no data is found above this value");
    assertEQ(higher_, undefined, "The treeSet should return undefined when no data is found below this value");
    assertEQ(lower, "ee", "The treeSet should return the correct lower value");
    assertEQ(higher, "gg", "The treeSet should return the correct higher value");
    assertEQ(treeSet.length, 8, "The treeSet length should be 8");
}

function testGetLowerValueGetHigherValueEmptyStringValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let lower = treeSet.getLowerValue("");
    let higher = treeSet.getHigherValue("");
    assertEQ(lower, undefined, "The treeSet should return undefined for the lower value in an empty set");
    assertEQ(higher, undefined, "The treeSet should return undefined for the higher value in an empty set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0")
}

function testGetLowerValueGetHigherValueDescendingOrder() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return firstValue.compareTo(secondValue) });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("d");
    treeSet.add("a");
    let lower = treeSet.getLowerValue("c");
    let higher = treeSet.getHigherValue("c");
    assertEQ(lower, "d", "The treeSet should return undefined for the lower value in an empty set");
    assertEQ(higher, "b", "The treeSet should return undefined for the higher value in an empty set");
    assertEQ(treeSet.length, 4, "The treeSet length should be 4")
}

function testGetLowerValueGetHigherValueObjectValues() {
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
    let lower_ = treeSet.getLowerValue(obj_trd);
    let higher = treeSet.getHigherValue(obj);
    let higher_ = treeSet.getHigherValue(obj_sec);
    let lower = treeSet.getLowerValue(obj_sec);
    assertTrue(equalObjects(lower!, obj), "The treeSet should return the correct lower value");
    assertTrue(equalObjects(lower_!, obj_sec), "The treeSet should return the correct lower value");
    assertTrue(equalObjects(higher!, obj_sec), "The treeSet should return the correct higher value");
    assertTrue(equalObjects(higher_!, obj_trd), "The treeSet should return the correct higher value");
    assertEQ(treeSet.length, 3, "The treeSet length should be 3")
}

function testGetLowerValueGetHigherValueObjectValuesBoundaryValues() {
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
    let lower = treeSet.getLowerValue(obj);
    let higher = treeSet.getHigherValue(obj_trd);
    assertEQ(lower, undefined, "The treeSet should return undefined for the lower value at the boundary value");
    assertEQ(higher, undefined, "The treeSet should return undefined for the higher value at the boundary value");
    assertEQ(treeSet.length, 3, "The treeSet length should be 3")
}
