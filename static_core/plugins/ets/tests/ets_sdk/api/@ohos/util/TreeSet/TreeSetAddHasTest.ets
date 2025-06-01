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
    const suite = new ArkTestsuite("TreeSet Add Has API tests")

    suite.addTest("TreeSet add data", testAddData)
    suite.addTest("TreeSet add data with same key and different values", testAddDataWithSameKeyDifferentValues)
    suite.addTest("TreeSet add empty string data", testAddEmptyStringData)
    suite.addTest("TreeSet has value", testHasValue)
    suite.addTest("TreeSet has not set value", testHasNonExistingValue)
    suite.addTest("TreeSet has value of the null TreeSet", testHasValueInEmptyTreeSet)
    suite.addTest("TreeSet has empty string value when treeSet is empty", testHasEmptyStringValueInEmptyTreeSet)
    suite.addTest("TreeSet has value after mix interface", testHasValueAfterMixedOperations)

    return suite.run()
}

class MyObject extends Object {
    size: number;
    constructor(size: number) {
        super();
        this.size = size;
    }
}

function testAddData() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result = treeSet.has("aa");
    assertEQ(result, true, "The treeSet should contain the added value");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function testAddDataWithSameKeyDifferentValues() {
    let treeSet: TreeSet<MyObject> = new TreeSet<MyObject>((firstValue: MyObject,
        secondValue: MyObject): number => { return secondValue.size - firstValue.size });
    let obj: MyObject = new MyObject(1);
    let obj2: MyObject = new MyObject(1);
    treeSet.add(obj);
    treeSet.add(obj2);
    let result = treeSet.has(obj);
    assertEQ(result, true, "The treeSet should contain the added value");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function testAddEmptyStringData() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return firstValue.compareTo(secondValue) });
    treeSet.add("");
    let result = treeSet.has("");
    assertEQ(result, true, "The treeSet should contain the added value");
    assertEQ(treeSet.length, 1, "The treeSet Length should be 1");
}

function testHasValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result = treeSet.has("aa");
    assertEQ(result, true, "The treeSet should has the value");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function testHasNonExistingValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result = treeSet.has("bb");
    assertEQ(result, false, "The treeSet should not has the value that is not set");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function testHasValueInEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let result = treeSet.has("aa");
    assertEQ(result, false, "The treeSet should not has the value when the treeSet is empty");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testHasEmptyStringValueInEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let result = treeSet.has("");
    assertEQ(result, false, "The treeSet should not has the value that is not set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testHasValueAfterMixedOperations() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("squirrel");
    let resultOne = treeSet.has("squirrel");
    assertEQ(resultOne, true, "The treeSet should has the value");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
    treeSet.add("aa");
    let resultTwo = treeSet.has("aa");
    assertEQ(resultTwo, true, "The treeSet should has the value");
    assertEQ(treeSet.length, 2, "The treeSet length should be 2");
    treeSet.remove("squirrel");
    let resultThree = treeSet.has("squirrel");
    assertEQ(resultThree, false, "The treeSet should not has the value that is not set");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
    treeSet.add("squirrel");
    treeSet.clear();
    let resultFour = treeSet.has("aa");
    assertEQ(resultFour, false, "The treeSet should not has the value that is not set");
    let resultFive = treeSet.has("squirrel");
    assertEQ(resultFive, false, "The treeSet should not has the value that is not set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}
