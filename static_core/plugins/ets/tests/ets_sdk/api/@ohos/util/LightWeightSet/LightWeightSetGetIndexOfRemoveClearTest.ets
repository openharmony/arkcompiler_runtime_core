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

import {LightWeightSet} from "@ohos.util.LightWeightSet";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("LightWeightSet GetIndexOf Remove Clear API tests")

    suite.addTest("LightWeightSet getIndexOf data return index",
        testGetIndexOfExistingData)
    suite.addTest("LightWeightSet getIndexOf not set data return index",
        testGetIndexOfNonExistingData)
    suite.addTest("LightWeightSet getIndexOf data return index when the LightWeightSet is empty",
        testGetIndexOfDataInEmptySet)
    suite.addTest("LightWeightSet getIndexOf complex data return index",
        testGetIndexOfComplexData)
    suite.addTest("LightWeightSet getIndexOf empty string data return index",
        testGetIndexOfEmptyStringData)
    suite.addTest("LightWeightSet remove value",
        testRemoveExistingValue)
    suite.addTest("LightWeightSet remove not set value",
        testRemoveNonExistingValue)
    suite.addTest("LightWeightSet remove value from the null lightWeightSet",
        testRemoveValueFromEmptySet)
    suite.addTest("LightWeightSet remove Max value",
        testRemoveMaxValue)
    suite.addTest("LightWeightSet clear data",
        testClearData)
    suite.addTest("LightWeightSet clear the null lightWeightSet",
        testClearEmptySet)

    return suite.run()
}

function testGetIndexOfExistingData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.getIndexOf("aa");
    let result1 = lightWeightSet.getIndexOf("bb");
    assertEQ(result, 0, "The lightWeightSet should return the correct index for the value");
    assertEQ(result1, 1, "The lightWeightSet should return the correct index for the value");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testGetIndexOfNonExistingData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.getIndexOf("cc");
    assertEQ(result, -3, "The lightWeightSet should return -3 that indicates where the key should be inserted");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testGetIndexOfDataInEmptySet() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    let result = lightWeightSet.getIndexOf("aa");
    assertEQ(result, -1, "The lightWeightSet should return -1 for a value in an empty set");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

class C1 {
    name: string = ""
    age: string = ""
}
function testGetIndexOfComplexData() {
    let lightWeightSet: LightWeightSet<object> = new LightWeightSet<object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    lightWeightSet.add(a);
    let result = lightWeightSet.getIndexOf(a);
    assertEQ(result, 0, "The lightWeightSet should return 0 for a value");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testGetIndexOfEmptyStringData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    let age = ''
    lightWeightSet.add(age);
    let result = lightWeightSet.getIndexOf(age);
    assertEQ(result, 0, "The lightWeightSet should return 0 for a value");
    assertEQ(lightWeightSet.length, 1, "LightWeightSet length should be 1");
}

function testRemoveExistingValue() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    let result = lightWeightSet.remove("aa");
    assertEQ(result, "aa", "The lightWeightSet should remove the value and return it");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testRemoveNonExistingValue() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    let result = lightWeightSet.remove("bb");
    assertEQ(result, undefined, "The lightWeightSet should return undefined for a value that is not set");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testRemoveValueFromEmptySet() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    let result = lightWeightSet.remove("aa");
    assertEQ(result, undefined, "The lightWeightSet should return undefined for a value in an empty set");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testRemoveMaxValue() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    const MY_MAX = 1024000;
    lightWeightSet.add(MY_MAX);
    let result = lightWeightSet.remove(MY_MAX);
    assertEQ(result, MY_MAX, "The lightWeightSet should return MY_MAX");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testClearData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    let result = lightWeightSet.isEmpty();
    lightWeightSet.clear();
    assertEQ(result, false, "The lightWeightSet should not be empty");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testClearEmptySet() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.clear();
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}
