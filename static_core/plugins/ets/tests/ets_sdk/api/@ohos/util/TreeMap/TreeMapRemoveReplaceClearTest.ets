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

import {TreeMap} from "api"

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("TreeMap Remove Replace Clear API tests")

    suite.addTest("TreeMap remove key", testRemoveExistingKey)
    suite.addTest("TreeMap remove not set key", testRemoveNonExistingKey)
    suite.addTest("TreeMap remove key from the null treeMap", testRemoveKeyFromEmptyTreeMap)
    suite.addTest("TreeMap replace value", testReplaceExistingValue)
    suite.addTest("TreeMap replace not set key", testReplaceNonExistingKey)
    suite.addTest("TreeMap replace value in the null treeMap", testReplaceValueInEmptyTreeMap)
    suite.addTest("TreeMap replace empty string when treeMap is empty", testReplaceEmptyStringInEmptyTreeMap)
    suite.addTest("TreeMap clear data", testClearData)
    suite.addTest("TreeMap clear the null treeMap", testClearEmptyTreeMap)

    return suite.run()
}

function testRemoveExistingKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("aa", "123");
    let result = treeMap.remove("aa");
    assertEQ(result, "123", "The treeMap should remove the element and return its value");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testRemoveNonExistingKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("aa", "123");
    let result = treeMap.remove("bb");
    assertEQ(result, undefined, "The treeMap should return undefined for a key that is not set");
    assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testRemoveKeyFromEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    let result = treeMap.remove("aa");
    assertEQ(result, undefined, "The treeMap should return undefined for a key in an empty map");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testReplaceExistingValue() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("aa", "123");
    let result1 = treeMap.replace("aa", "456");
    let result2 = treeMap.get("aa");
    assertEQ(result1, true, "The treeMap should replace the value for the key");
    assertEQ(result2, "456", "The treeMap should return the new value for the key");
    assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testReplaceNonExistingKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("aa", "123");
    let result1 = treeMap.replace("bb", "456");
    let result2 = treeMap.get("aa");
    assertEQ(result1, false, "The treeMap should return false for a key that is not set");
    assertEQ(result2, "123", "The treeMap should return the new value for the key");
    assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testReplaceValueInEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    let result = treeMap.replace("aa", "789");
    assertEQ(result, false, "The treeMap should return false for a key in an empty map");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testReplaceEmptyStringInEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    let result = treeMap.replace("", "789");
    assertEQ(result, false, "The treeMap should return false for a key in an empty map");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testClearData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("aa", "123");
    let result = treeMap.isEmpty();
    treeMap.clear();
    assertEQ(result, false, "The treeMap should not be empty");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testClearEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.clear();
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}
