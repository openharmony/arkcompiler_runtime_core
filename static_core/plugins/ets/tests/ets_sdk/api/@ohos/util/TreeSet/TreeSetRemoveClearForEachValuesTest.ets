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

import {TreeSet} from "api"

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("TreeSet Remove Clear ForEach Values API tests")

    suite.addTest("TreeSet remove value",
        testRemoveExistingValue)
    suite.addTest("TreeSet remove not set value",
        testRemoveNonExistingValue)
    suite.addTest("TreeSet remove value from the null treeSet",
        testRemoveValueFromEmptyTreeSet)
    suite.addTest("TreeSet remove empty string value when treeSet is empty",
        testRemoveEmptyStringValueFromEmptyTreeSet)
    suite.addTest("TreeSet clear data",
        testClearData)
    suite.addTest("TreeSet clear the null treeSet",
        testClearEmptyTreeSet)
    suite.addTest("TreeSet add data after clear the TreeSet",
        testAddDataAfterClearTreeSet)
    suite.addTest("TreeSet forEach the treeSet with no reference",
        testForEachNoReference)
    suite.addTest("TreeSet forEach the treeSet with reference is value",
        testForEachValueReference)
    suite.addTest("TreeSet forEach the treeSet with reference are value and key",
        testForEachValueKeyReference)
    suite.addTest("TreeSet forEach the treeSet with three references",
        testForEachThreeReferences)
    suite.addTest("TreeSet values return iterator by value",
        testValuesReturnIteratorByValue)
    suite.addTest("TreeSet values return iterator by value with null treeSet",
        testValuesReturnIteratorByValueWithEmptyTreeSet)

    return suite.run()
}

function testRemoveExistingValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result1 = treeSet.remove("aa");
    let result2 = treeSet.has("aa");
    assertEQ(result1, true, "The treeSet should remove the value");
    assertEQ(result2, false, "The treeSet should not has the value");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testRemoveNonExistingValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result = treeSet.remove("bb");
    assertEQ(result, false, "The treeSet should return false for the value that is not set");
    assertEQ(treeSet.length, 1, "The treeSet length should be 1");
}

function testRemoveValueFromEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let result = treeSet.remove("aa");
    assertEQ(result, false, "The treeSet should return undefined for a value in an empty set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testRemoveEmptyStringValueFromEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let result = treeSet.remove("");
    assertEQ(result, false, "The treeSet should return undefined for a value in an empty set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testClearData() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.clear();
    let result1 = treeSet.has("aa");
    let result2 = treeSet.isEmpty();
    assertEQ(result1, false, "The treeSet should not has the value");
    assertEQ(result2, true, "The treeSet should be empty");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testClearEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.clear();
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testAddDataAfterClearTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return firstValue.compareTo(secondValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.clear();
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
    treeSet.add("cc");
    treeSet.add("dd");
    treeSet.add("ee");
    assertEQ(treeSet.length, 3, "The treeSet length should be 3");
    assertEQ(treeSet.getFirstValue(), "ee", "The treeMap first value should be ee");
    assertEQ(treeSet.getLastValue(), "cc", "The treeMap last value should be cc");
}

function testForEachNoReference() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let count: number = 0;
    treeSet.forEach(() => {
        count++;
    });
    assertEQ(count, 2, "The treeSet forEach should iterate over all elements");
}

function testForEachValueReference() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let flag = false;
    treeSet.forEach((value: string) => {
        flag = treeSet.has(value)
        assertEQ(flag, true, "The treeSet forEach should iterate over all values");
    });
}

function testForEachValueKeyReference() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let flag = false;
    treeSet.forEach((value: string, key: string) => {
        flag = treeSet.has(value)
        assertEQ(flag, true, "The treeSet forEach should iterate over all values and keys");
    });
}

function testForEachThreeReferences() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let flag = false;
    treeSet.forEach((value: string, key: string, treeSet: TreeSet<string>) => {
        flag = treeSet.has(value)
        assertEQ(flag, true, "The treeSet forEach should iterate over all values, keys, and the set itself");
    });
}

function testValuesReturnIteratorByValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let it = treeSet.values();
    assertEQ(it.next().value, "aa", "The treeSet values iterator should return the first value");
    assertEQ(it.next().value, "bb", "The treeSet values iterator should return the second value");
    assertEQ(it.next().value, undefined, "The treeSet values iterator should return undefined for no more values");
}

function testValuesReturnIteratorByValueWithEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let it = treeSet.values();
    assertEQ(it.next().value, undefined, "The treeSet values iterator should return undefined when treeSet is empty");
}
