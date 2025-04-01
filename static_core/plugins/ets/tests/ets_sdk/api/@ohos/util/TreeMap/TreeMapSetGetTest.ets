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
    const suite = new ArkTestsuite("TreeMap Set Get API tests")

    suite.addTest("TreeMap set data", testSetData)
    suite.addTest("TreeMap set empty string data", testSetEmptyStringData)
    suite.addTest("TreeMap loop set data", testLoopSetData)
    suite.addTest("TreeMap get data", testGetData)
    suite.addTest("TreeMap get not set data", testGetNonExistingData)
    suite.addTest("TreeMap get data from the null treeMap", testGetDataFromEmptyTreeMap)
    suite.addTest("TreeMap get empty string data when treeMap is mepty", testGetEmptyStringDataFromEmptyTreeMap)

    return suite.run()
}

function testSetData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("a", "123");
    let result = treeMap.get("a");
    assertEQ(result, "123", "The treeMap should get the value by key");
    assertEQ(treeMap.length, 1, "The treeMap Length should be 1");
}

function testSetEmptyStringData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("", "123");
    let length = treeMap.length;
    let result = treeMap.get("");
    assertEQ(result, "123", "The treeMap should get the value by key");
    assertEQ(treeMap.length, 1, "The treeMap Length should be 1");
}

function testLoopSetData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    let arr = 'abc';
    for (let i = 0; i < 1000; i++) {
        treeMap.set(arr + i, "123");
    }
    let length = treeMap.length;
    for (let k = 0; k < 1000; k++) {
        let result = treeMap.get(arr + k);
        assertEQ(result, "123", "The treeMap should get the value by key");
    }
    assertEQ(length, 1000, "The treeMap Length should be 1");

}

function testGetData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("aa", "123");
    let result = treeMap.get("aa");
    assertEQ(result, "123", "The treeMap should return the correct value for the key");
    assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testGetNonExistingData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    treeMap.set("aa", "123");
    let result = treeMap.get("bb");
    assertEQ(result, undefined, "The treeMap should return undefined for the key that is not set");
    assertEQ(treeMap.length, 1, "The treeMap length should be 1");
}

function testGetDataFromEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    let result = treeMap.get("aa");
    assertEQ(result, undefined, "The treeMap should return undefined for the key in an empty map");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testGetEmptyStringDataFromEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): boolean => {
            return firstValue < secondValue
        });
    let result = treeMap.get("");
    assertEQ(result, undefined, "The treeMap should return undefined for the key in an empty map");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}
